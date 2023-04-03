#include <unistd.h>
#include <json-c/json.h>

void checkConfig() {
    int configIssues = 0;

    if (access("config.json", F_OK) != 0) {
        printf("FATAL: Config file not found\n");
        configIssues = 1;
    }

    if (configIssues == 0) {
        struct json_object * configFile;
        configFile = json_object_from_file("config.json");


        if (access(json_object_get_string(json_object_object_get(configFile, "subredditsFile")), F_OK) != 0) {
            printf("FATAL: Subreddits file \"%s\" not found.\n", json_object_get_string(json_object_object_get(configFile, "subredditsFile")));
            configIssues ++;
        }

        if (access(json_object_get_string(json_object_object_get(configFile, "downloadsDirectory")), F_OK) != 0) {
            printf("FATAL: Downloads directory \"%s\" not found.\n", json_object_get_string(json_object_object_get(configFile, "downloadsDirectory")));
            configIssues ++;
        }

        json_object_put(configFile);
    }

    if (configIssues == 0) {
            printf("No configuration errors detected.\n");
    } else {
        printf("Fatal error(s) detected. Aborting.\n");
        if (configIssues > 1) {
            printf("The config file may be misformatted, please check \"config.json\".\n");
        }
        exit(0);
    }
}