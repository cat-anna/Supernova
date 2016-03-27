/*
 * ErrorCodes.h
 *
 * Error codes definition for Supernova
 *
 *  Created on: 12-03-2012
 *      Author: Admin
 */

#ifndef ERRORCODES_H_
#define ERRORCODES_H_

enum{
	NO_ERROR					= 0,
	SUCCES						= NO_ERROR,

	ERRORCODE_FATAL 			= 1,
	ERRORCODE_WRONG_INPUT		= 2,//input data is illegal, ie empty pointer

	ERRORCODE_NO_MORE_MEMORY	= ERRORCODE_FATAL,
	ERRORCODE_IPC_IS_FULL,

	EC_WRONG_HANDLE,

//io operations
	EC_IO_UNKNOWN				= NO_ERROR + 0x100,
	EC_READ_OUT_OF_BOUND,
	EC_READ_NOT_ALLOWED,


	EC_SHARED_UNKNOWN			= (2 << 16),
	EC_SHARED_NOTFOUND,
	EC_SHARED_FUNCION_NOTFOUND,
	EC_SHARED_SYMBOL_NOT_FOUND,


	EC_WRONG_EXEC_TYPE			= (3 << 16),
	EC_EXEC_NOT_DYNAMIC,
	//EC_EXEC_

	EC_DRIVER_DEPEND_UNKNOWN	= (4 << 16),
	EC_FUNCTION_NOT_SUPPORTED,
	EC_NO_SUCH_DEVICE,
};

//global device errors

enum {
	DEV_EC_UNKNOWN				= (5 << 16),
	DEV_EC_SEEK,
};

//------------------------------SYSTEM-ERRORCODES------------------------------------

enum{
	EC_SYSTEM_FATAL				= (1 << 16),
	EC_IRQ_NOT_AVAILABLE,
};


#endif /* ERRORCODES_H_ */
