#include <stdio.h> // fprintf, printf, fopen, fclose, fwrite
#include <stdlib.h> // malloc, free, abort
#include <string.h> // strerror
#include <stdbool.h> // bool, true, false
#include <errno.h> // errno
#include <sys/stat.h> // stat

#if !defined(LINE_MAX)
#define LINE_MAX 4096
#endif

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

#if defined(_WIN32)
#include "dirent_windows.h" // opendir, readdir, closedir
#else
#include <dirent.h> // opendir, readdir, closedir
#endif

static void *xmalloc(size_t length) {
	void *data = malloc(length);
	if (!data) {
		fprintf(stderr, "Out of memory\n");
		abort();
	}
	return data;
}

// Like fgets but for strings and ignores the newline
static char *sgets(char *str, size_t length, char **input) {
	char *next = *input;
	size_t read = 0;
	while (read + 1 < length && *next) {
		if (*next == '\n') {
			next++; // skip newline
			read++;
			break;
		}
		*str++ = *next++;
		read++;
	}
	if (read == 0) {
		return 0;
	}
	*str = '\0';
	*input = next;
	return str;
}

static int usage(const char *app) {
	fprintf(stderr, "Usage: %s [games directory]\n", app);
	return 0;
}

static bool check_for_games(const char *path) {
	// Checks if the games path exists
	struct stat s;
	if (stat(path, &s) == 0 && S_ISDIR(s.st_mode)) {
		return true;
	}
	return false;
}

static bool is_game_path(const char *games_path, const char *game_path) {
	// Checks if the game path contains a game
	char path[PATH_MAX];
	snprintf(path, sizeof path, "%s/%s/%s.desktop", games_path, game_path, game_path);
	struct stat s;
	if (stat(path, &s) == 0 && S_ISREG(s.st_mode)) {
		return true;
	}
	return false;
}

static bool apply_retroarch_to_game(const char *games_path, const char *game_path) {
	char old_path[PATH_MAX];
	char new_path[PATH_MAX];
	snprintf(old_path, sizeof old_path, "%s/%s/%s.desktop", games_path, game_path, game_path);
	snprintf(new_path, sizeof new_path, "%s/%s/%s.patched", games_path, game_path, game_path);

	FILE *old_fp = fopen(old_path, "rb");
	if (!old_fp) {
		fprintf(stderr, "Failed to open game '%s' configuration for reading [%s]\n", game_path, strerror(errno));
		return false;
	}

	FILE *new_fp = fopen(new_path, "wb");
	if (!new_fp) {
		fprintf(stderr, "Failed to open game '%s' configuration for writing [%s]\n", game_path, strerror(errno));
		fclose(old_fp);
		return false;
	}

	// Read the old configuration into memory
	fseek(old_fp, 0, SEEK_END);
	long length = ftell(old_fp);
	fseek(old_fp, 0, SEEK_SET);
	char *contents = (char *)xmalloc(length);

	if (fread(contents, length, 1, old_fp) != 1) {
		fprintf(stderr, "Failed to read game '%s' configuration [%s]\n", game_path, strerror(errno));
		fclose(old_fp);
		fclose(new_fp);
		return false;
	}

	// The old configuration is in memory now, no longer need this stream
	fclose(old_fp);

	// Consume every line of the configuration and patch it with --retroarch
	char line[LINE_MAX];
	char *consume = contents;
	bool patched = false;
	while (sgets(line, sizeof line, &consume)) {
		if (!strncmp(line, "Exec=", 5)) {
			if (strstr(line, "--retroarch")) {
				patched = false;
			} else {
				// Now append the --retroarch command line to this line
				strcat(line, " --retroarch");
				patched = true;
			}
		} else if (!strncmp(line, "Name=", 5)) {
			// Output the name of the game that was just patched
			if (patched) {
				printf("Patched game '%s' which is '%s'\n", game_path, line + 5);
			} else {
				printf("Ignored game '%s' which is '%s' since it's already patched\n", game_path, line + 5);
			}
		}
		// Write it out to the new file
		fwrite(line, strlen(line), 1, new_fp);

		// Write out a newline character
		char new_line = '\n';
		fwrite(&new_line, 1, 1, new_fp);
	}

	// The new configuration is written, no longer need this stream
	fclose(new_fp);

	// The new configuration is written, no longer need the old configuration in memory
	free(contents);

	// The new configuration is written, no longer need the old file
	remove(old_path);

	// Busy-wait for the old file to be deleted as some operating systems,
	// like Windows remove is not atomic
	for (;;) {
		FILE *fp = fopen(old_path, "r");
		if (!fp) {
			// Cannot be opened now, it's deleted
			break;
		}
		fclose(fp);
	}

	// Renames are atomic on all operating systemss
	return rename(new_path, old_path) == 0;
}

static bool apply_retroarch_to_games(const char *path) {
	DIR *dir = opendir(path);
	if (!dir) {
		return false;
	}

	struct dirent *ent = 0;
	int games = 0;
	int patched = 0;
	while ((ent = readdir(dir))) {
		// Ignore anything that doesn't look like a game path
		if (ent->d_type != DT_DIR || !is_game_path(path, ent->d_name)) {
			continue;
		}

		games++;

		if (apply_retroarch_to_game(path, ent->d_name)) {
			patched++;
		}
	}

	closedir(dir);

	if (games) {
		printf("Patched %d of %d games\n", patched, games);
	} else {
		printf("No games found\n");
	}

	return games == patched;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		return usage(argv[0]);
	}

	if (!check_for_games(argv[1])) {
		fprintf(stderr, "Could not find games directory '%s' [%s]\n", argv[1], strerror(errno));
		return 0;
	}

	return apply_retroarch_to_games(argv[1]);
}
