/*
 * LocalCppObj.hpp
 *
 *  Created on: 12 July 2023 - May 26, 2024
 *      Author: jeff Hertig
 *
 *      The whole point of this is to create a local copy of the shared memory section so you can easily monitor changes in the shared memory.   Calling checkChanges will synchronize the data and give you a list of changes.
 *
 *      This is just a way to have a local copy of the data for read access.  If you want to write to it then write directly to the CObj.
 */

#ifndef LOCALCPPOBJ_HPP_
#define LOCALCPPOBJ_HPP_

#include <stdlib.h>
#include <stdio.h>

#include "CppON.hpp"
#include "SCppObj.hpp"


typedef struct LOCAL_CPP_OBJ
{
	struct LOCAL_CPP_OBJ	*subs;
	STRUCT_LISTS			*obj;
	void	 				*localObj;
	uint32_t				hysteresis;
} LOCAL_CPP_OBJ;


class LocalCppObj
{
public:
					LocalCppObj( SCppObj  *parent );
					virtual ~LocalCppObj();

	LOCAL_CPP_OBJ	*resolveName( const char *path, LOCAL_CPP_OBJ *obj = NULL );
	void			update( LOCAL_CPP_OBJ *obj = NULL );
	void			update( const char *path, LOCAL_CPP_OBJ *obj = NULL ){ LOCAL_CPP_OBJ *lObj = resolveName( path, obj ); if( lObj ) { update( lObj ); } }
	bool			checkChanges( CppON *rst, LOCAL_CPP_OBJ *obj = NULL );
	bool			checkChanges( const char *path, CppON *rst, LOCAL_CPP_OBJ *obj = NULL );
	SCppObj			*parent( void ) { return shared; }

private:
	void			deleteSub( LOCAL_CPP_OBJ &lObj );
	void 			addSub( LOCAL_CPP_OBJ *lObj, STRUCT_LISTS &target );
	void			*basePtr;
	LOCAL_CPP_OBJ	*root;
	SCppObj			*shared;
};

#endif /* LOCALCPPOBJ_HPP_ */
