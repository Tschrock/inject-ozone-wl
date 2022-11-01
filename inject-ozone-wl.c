/*
 * Hooks main() using LD_PRELOAD, so we can add the `--ozone-platform=wayland`
 * flag to electron processes. USE AT YOUR OWN RISK.
 *
 * Based on this example file:
 * https://gist.github.com/apsun/1e144bf7639b22ff0097171fa0f8c6b1
 * 
 * Compilation:
 * `gcc inject-ozone-wl.c -o inject-ozone-wl.so -fPIC -shared -ldl`
 * 
 * Instalation:
 * `sudo cp inject-ozone-wl.so /usr/local/lib/`
 *
 * Usage:
 * `LD_PRELOAD=/usr/local/lib/inject-ozone-wl.so myprogram'
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

/* Trampoline for the real main() */
static int (*main_orig)(int, char **, char **);

// Names of processes to append arguments to
static char * ozone_names[] = {
    "code",
    "code-insiders",
    "obsidian",
    "discord",
    "discord-canary",
    "Discord",
    "DiscordCanary",
    "blockbench",
    "drawio"
};
static int ozone_names_len = sizeof(ozone_names) / sizeof(ozone_names[0]);

int match_name(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    if (suffix_len == str_len) return strcmp(str, suffix) == 0;
    return (strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0)
        && (str[str_len - suffix_len - 1] == '/');
}

int match_arg_start(const char *prefix, const char *str) {
    size_t prefix_len = strlen(prefix);
    return strncmp(str, prefix, prefix_len) == 0;
}

int match_str(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

int arr_matches(const char *str, char ** arr, size_t arr_len, int (*matcher)(const char *, const char *)) {
    for(int i = 0; i < arr_len; ++i) {
        if(matcher(str, arr[i])) return 1;
    }
    return 0;
}

/* Our fake main() that gets called by __libc_start_main() */
int main_hook(int argc, char **argv, char **envp)
{
    char **newv = 0;
    if(argc > 0) {
        if(arr_matches(argv[0], ozone_names, ozone_names_len, match_name)) {
            if(arr_matches("--ozone-platform=", argv, argc, match_arg_start)) {
                // Ozone platform already added
            } else if(arr_matches("--type=", argv, argc, match_arg_start)) {
                // Don't need to add it to sub-processes
            } else if(arr_matches("-p", argv, argc, match_str)) {
                // Fixes environment error in vscode
            } else {
                // Set the ozone platform
                newv = malloc((argc + 3) * sizeof(*newv));
                memmove(newv, argv, sizeof(*newv) * argc);
                newv[argc] = "--ozone-platform=wayland";
                newv[argc+1] = "--enable-features=WaylandWindowDecorations";
                newv[argc+2] = 0;
                argc++;
                argc++;
                argv = newv;
            }
        }
    }
    int ret = main_orig(argc, argv, envp);
    free(newv);
    return ret;
}

/*
 * Wrapper for __libc_start_main() that replaces the real main
 * function with our hooked version.
 */
int __libc_start_main(
    int (*main)(int, char **, char **),
    int argc,
    char **argv,
    int (*init)(int, char **, char **),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end)
{
    /* Save the real main function address */
    main_orig = main;

    /* Find the real __libc_start_main()... */
    typeof(&__libc_start_main) orig = dlsym(RTLD_NEXT, "__libc_start_main");

    /* ... and call it with our custom main function */
    return orig(main_hook, argc, argv, init, fini, rtld_fini, stack_end);
}
