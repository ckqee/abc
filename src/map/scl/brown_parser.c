#include<tcl/tcl.h>
#include<stdio.h>

int parsedCreateClockCallback(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
    /* Don't change the order of the following argvs */
    char* period = argv[1];
    char* portPin = argv[2];
    char* name = argv[3];
    /* Don't change the order of the above argvs */

    // TODO: replace the below code with setting the actual constraints in abc
    printf("Period: %s\n", period);
    printf("Port Pin: %s\n", portPin);
    printf("Name: %s\n", name);
    printf("-----------------\n");

    return TCL_OK;
 }
 int parsedSetInputDelayCallback(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
    /* Don't change the order of the following argvs */
    char* portPin = argv[1];
    char* clk = argv[2];
    char* delayValue = argv[3];
    /* Don't change the order of the above argvs */

    // TODO: replace the below code with setting the actual constraints in abc
    printf("Port Pin: %s\n", portPin);
    printf("Clock: %s\n", clk);
    printf("Delay Value: %s\n", delayValue);
    printf("-----------------\n");

    return TCL_OK;
 }
 int parsedSetMaxFanoutCallback(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
    /* Don't change the order of the following argvs */
    char* portPin = argv[1];
    char* clk = argv[2];
    char* delayValue = argv[3];
    /* Don't change the order of the above argvs */

    // TODO: replace the below code with setting the actual constraints in abc
    printf("Port Pin: %s\n", portPin);
    printf("Clock: %s\n", clk);
    printf("Delay Value: %s\n", delayValue);
    printf("-----------------\n");

    return TCL_OK;
 }
 int parsedSetMaxTransitionCallback(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
    /* Don't change the order of the following argvs */
    char* portPin = argv[1];
    char* clk = argv[2];
    char* delayValue = argv[3];
    /* Don't change the order of the above argvs */

    // TODO: replace the below code with setting the actual constraints in abc
    printf("Port Pin: %s\n", portPin);
    printf("Clock: %s\n", clk);
    printf("Delay Value: %s\n", delayValue);
    printf("-----------------\n");

    return TCL_OK;
 }

 /* TODO: define another callback function for another command we will support */
 int parseCOMMAND_NAME_CALLBACK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
    /* Don't change the order of the following argvs */
    // TODO: define what variables to read from the tcl script


    /* Don't change the order of the above argvs */

    return TCL_OK;
 }


int main(){
    char parseCommand[100];
    strcpy(parseCommand, "sdc::parse_file ");
    
    // TODO: SDC file path [replace with the sdc file name from the flag]
    char* sdcFile = "jpeg_encoder.sdc";
    strcat(parseCommand, sdcFile);

    // initialize the tcl interpreter
    Tcl_Interp *interp;
    interp = Tcl_CreateInterp();
    if(interp == NULL){
        fprintf(stderr, "Error in Tcl_CreateInterp, aborting\n");
        return -1;
    }
    if(Tcl_Init(interp) == TCL_ERROR){
        fprintf(stderr, "Error in Tcl_Init: %s\n", Tcl_GetStringResult(interp));
        return -1;
    }
    // register the defined function with the tcl interpreter
    Tcl_CreateCommand(interp, "parsedSetInputDelayCallback", parsedSetInputDelayCallback, (ClientData) NULL, (ClientData) NULL);
    Tcl_CreateCommand(interp, "parsedCreateClockCallback", parsedCreateClockCallback, (ClientData) NULL, (ClientData) NULL);

    // TODO: register other defined functions here
    // TODO: example: Tcl_CreateCommand(interp, "parseCOMMAND_NAME_CALLBACK", parseCOMMAND_NAME_CALLBACK, (ClientData) NULL, (ClientData) NULL);

    // No need to change any of the below code
    int code = Tcl_EvalFile(interp, "brown_parser.tcl");
    if(code == TCL_OK){
        Tcl_Eval(interp, parseCommand);
    } else
    {
        printf("SDC File Parsing Failed!\n");
    }
    printf("SDC File Parsing Done!\n");
    return 0;
}
