#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char x[2];
        
    while(1){
        printf("Enter 1 to simulate alt tab\n");
        printf("Enter 2 to simulate page down\n");
        printf("Enter 3 to simulate page up\n");
        printf("Enter 4 to quit\n");
        scanf("%s", x);
        if(strcmp(x,"1") == 0){
            system("xdotool key alt+Tab");
        } else if(strcmp(x,"2") == 0){
            system("xdotool key alt+Tab");
            system("xdotool key Next");
        }  else if(strcmp(x,"3") == 0){
            system("xdotool key alt+Tab");
            system("xdotool key Prior");
        }  else if(strcmp(x,"4") == 0){
            return;
        } 


    }


    
}
