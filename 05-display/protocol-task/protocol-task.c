#include "protocol-task.h"
#include "stdio.h"
#include "pico/stdlib.h"
#include "string.h"

static api_t* api = {0};
static int commands_count = 0;

void protocol_task_init(api_t* device_api){
    api=device_api;
    for(int i=0;;i++){
        if (api[i].command_name!=NULL) commands_count++;
        else {
            commands_count++;
            break;
        }
    }
}

void protocol_task_handle(char* command_string){
    if (command_string==NULL)
        return;

    const char* command_name = command_string;
    const char* command_args = NULL;

    char* space_symbol = strchr(command_string, ' ');
    if (space_symbol)
    {
        *space_symbol = '\0';
        command_args = space_symbol + 1;
    }
    else
    {
        command_args = "";
    }
    // printf("Command found: ");
    // for(int i=0;;i++){
    //     if(command_string[i]!=' ') printf(command_string[i]);
    //     else {
    //         printf(". ");
    //         break;
    //     }
    // }
    // printf("Args found: ");
    // for (int i=0;;i++){
    //     if(command_args=="") break;
    //     if(command_args[i]!=' ') printf(command_args[i]);
    //     else{
    //         printf(".\n");
    //         break;
    //     }
    // }

    for(int i=0;i<commands_count;i++){
        if (!strcmp(command_name,api[i].command_name)){
            printf("Callback\n");
            api[i].command_callback(command_args);
            break;
        }
        if (i==commands_count-1) printf("Error, command not found.\n");
    }
}
