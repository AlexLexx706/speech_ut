#ifndef BASEPLUGINSERVER_COMMAND_H
#define BASEPLUGINSERVER_COMMAND_H


enum BasePluginServerCommand
{
	CmdExitServer = 0,
	CmdGetMarking,
	CmdBatchProcessing,
	CmdEndFileProcessing,
	CmdStopProcessing,
	CmdStartFileProcessing,
};


#endif //BASEPLUGINSERVER_COMMAND_H