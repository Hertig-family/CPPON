/*
 * LocalCppObj.cpp
 *
 *  Created on: 12 July 2023 - May 26, 2024
 *      Author: jeff Hertig
 */

#include "LocalCppObj.hpp"


/*
 * resolveName is much like the GetElement method of the CObj class (in fact I don't know why it is different) but it returns either "NULL" or a pointer to the structure/object you were looking for.
 * It's use is to loat a sub-structure of data to improve access performance.
 */

LOCAL_CPP_OBJ *LocalCppObj::resolveName( const char *path, LOCAL_CPP_OBJ *obj )
{
	if( path )
	{
		if( ! obj )
		{
			obj = root;
		}
		STRUCT_LISTS *base = obj->obj;
		if( base  && base->names )
		{
			const char *e;
			const char *key;
			for( e = path; *e && '/' != *e && '.' != *e; e++ );
			unsigned l = e - path;
			std::string s( path, l );

			for( unsigned i = 0; ( key = base->names[ i ] ); i += 2 )
			{
				unsigned j;
				for( j = 0; j < l && key[ j ] && path[ j ] == key[ j ]; j++);
				if( ! key[ j ] )																			// found
				{
					if( l != strlen( base->names[ i + 1 ] ) )												// Fail! Strings aren't same length
					{
					} else if( strncmp( base->names[ i + 1 ], path, l ) ) {									// Fail! Strings aren't equal
						break;
					} else if( *e ) {
						return resolveName( ++e, &(obj->subs[ i / 2 ] ) );									// Search for child
					} else {
						return &( obj->subs[ i / 2 ] );														// Found IT!
					}
				} else if( key[ j ] > path[ j ] ) {															// Fail!  We passed it without finding it.
					break;
				}
			}
		}
	}
	return NULL;
}

void LocalCppObj::update( LOCAL_CPP_OBJ *objIn )
{
	if( ! objIn )
	{
		objIn = root;
	}
	STRUCT_LISTS	*fromObj = objIn->obj;

	switch ( fromObj->type )
	{
		case SL_TYPE_DOUBLE:
			{
				shared->waitSem( fromObj->sem );
				*((double*)(objIn->localObj ) ) = *(double*)(shared->pointer( fromObj ) );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_INT64:
			{
				shared->waitSem( fromObj->sem );
				*((int64_t*)(objIn->localObj ) ) =  *(int64_t*)(shared->pointer( fromObj ) );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_INT32:
			{
				shared->waitSem( fromObj->sem );
				*((int32_t*)(objIn->localObj ) ) = *(int32_t*)(shared->pointer( fromObj ) );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_INT16:
			{
				shared->waitSem( fromObj->sem );
				*((uint16_t*)(objIn->localObj ) ) = *(uint16_t*)(shared->pointer( fromObj ) );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_INT8:
			{
				shared->waitSem( fromObj->sem );
				*((uint8_t*)(objIn->localObj ) ) = *(uint8_t*)(shared->pointer( fromObj ) );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_BOOL:
			{
				shared->waitSem( fromObj->sem );
				*((uint8_t*)(objIn->localObj ) ) = *(uint8_t*)(shared->pointer( fromObj ) );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_CHAR:
			{
				shared->waitSem( fromObj->sem );
				strncpy( ((char *)(objIn->localObj ) ), (const char *)(shared->pointer( fromObj ) ),  fromObj->size );
				sem_post( fromObj->sem );
			}
			break;
		case SL_TYPE_UNIT:
			{
				for( unsigned n = 0; fromObj->nSubs > n; n++ )
				{
					update( &objIn->subs[ n ] );
				}
			}
			break;
		case SL_TYPE_ARRAY:
			{
				for( unsigned n = 0; fromObj->nSubs > n; n++ )
				{
					update( &objIn->subs[ n ] );
				}
			}
			break;
		default:
			break;
	}
}

bool LocalCppObj::checkChanges( const char *path, CppON *rst, LOCAL_CPP_OBJ *obj )
{
	LOCAL_CPP_OBJ 	*lObj = resolveName( path, obj );
	bool		changes = false;
	if( lObj )
	{
		changes = checkChanges( rst, lObj );
	}
	return changes;
}

/*
 * Find changes in the data and report them in a COMap
 */
bool LocalCppObj::checkChanges( CppON *rtn, LOCAL_CPP_OBJ *objIn )
{
	bool 				isMap;
	bool				changes = false;

	if( ! objIn )
	{
		objIn = root;
	}

	if( ( isMap  = CppON::isMap( rtn ) ) || CppON::isArray( rtn )  )
	{
		STRUCT_LISTS	*fromObj = objIn->obj;
		switch ( fromObj->type )
		{
			case SL_TYPE_DOUBLE:
				{
					double hyst = ((double) objIn->hysteresis) / 100.0;
					shared->waitSem( fromObj->sem );
					double dShare = *(double*)(shared->pointer( fromObj ) );
					sem_post( fromObj->sem );
					double dSave = *((double*)(objIn->localObj ) );
					if( dShare > ( dSave + hyst) || dShare < (dSave - hyst ) )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new CODouble( *((double*)(objIn->localObj ) ) = dShare ) );
						} else {
							((COArray * )rtn )->append( new CODouble( *((double*)(objIn->localObj ) ) = dShare ) );
						}
					}
				}
				break;
			case SL_TYPE_INT64:
				{
					int64_t hyst = (int64_t)( objIn->hysteresis );
					shared->waitSem( fromObj->sem );
					int64_t iShare = *(int64_t*)(shared->pointer( fromObj ) );
					sem_post( fromObj->sem );
					int64_t iSave = *((int64_t*)(objIn->localObj ) );
					if( iShare > ( iSave + hyst) || iShare < (iSave - hyst ) )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new COInteger( (uint64_t)( *((int64_t*)(objIn->localObj ) ) = iShare ) ) );
						} else {
							((COArray * )rtn )->append( new COInteger( (uint64_t)(*((int64_t*)(objIn->localObj ) ) = iShare ) ) );
						}
					}
				}
				break;
			case SL_TYPE_INT32:
				{
					int32_t hyst =  objIn->hysteresis;
					shared->waitSem( fromObj->sem );
					int32_t iShare = *(int32_t*)(shared->pointer( fromObj ) );
					sem_post( fromObj->sem );
					int32_t iSave = *((int32_t*)(objIn->localObj ) );
					if( iShare > ( iSave + hyst) || iShare < (iSave - hyst ) )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new COInteger( (int)( *((int32_t*)(objIn->localObj ) ) = iShare ) ) );
						} else {
							((COArray * )rtn )->append( new COInteger( (int)(*((int32_t*)(objIn->localObj ) ) = iShare ) ) );
						}
					}
				}
				break;
			case SL_TYPE_INT16:
				{
					int32_t hyst =  objIn->hysteresis;
					shared->waitSem( fromObj->sem );
					int32_t iShare = (int32_t) *(uint16_t*)(shared->pointer( fromObj ) );
					sem_post( fromObj->sem );
					int32_t iSave = (int32_t) *((uint16_t*)(objIn->localObj ) );
					if( iShare > ( iSave + hyst) || iShare < (iSave - hyst ) )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new COInteger( (int)( *((uint16_t*)(objIn->localObj ) ) = (uint16_t) iShare ) ) );
						} else {
							((COArray * )rtn )->append( new COInteger( (int)( *((uint16_t*)(objIn->localObj ) ) = (uint16_t) iShare ) ) );
						}
					}
				}
				break;
			case SL_TYPE_INT8:
				{
					int32_t hyst =  objIn->hysteresis;
					shared->waitSem( fromObj->sem );
					int32_t iShare = (int32_t) *(uint8_t*)(shared->pointer( fromObj ) );
					sem_post( fromObj->sem );
					int32_t iSave = (int32_t) *((uint8_t*)(objIn->localObj ) );
					if( iShare > ( iSave + hyst) || iShare < (iSave - hyst ) )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new COInteger( (int)( *((uint8_t*)(objIn->localObj ) ) = (uint8_t) iShare ) ) );
						} else {
							((COArray * )rtn )->append( new COInteger( (int)( *((uint8_t*)(objIn->localObj ) ) = (uint8_t) iShare ) ) );
						}
					}
				}
				break;
			case SL_TYPE_BOOL:
				{
					shared->waitSem( fromObj->sem );
					int32_t iShare = (int32_t) *(uint8_t*)(shared->pointer( fromObj ) );
					sem_post( fromObj->sem );
					int32_t iSave = (int32_t) *((uint8_t*)(objIn->localObj ) );
					if( iSave != iShare )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new COBoolean(  0 != ( *((uint8_t*)(objIn->localObj ) ) = (uint8_t) iShare ) ) );
						} else {
							((COArray * )rtn )->append( new COBoolean(  0 != ( *((uint8_t*)(objIn->localObj ) ) = (uint8_t) iShare ) ) );
						}
					}
				}
				break;
			case SL_TYPE_CHAR:
				{
					shared->waitSem( fromObj->sem );
					const char *sShare = (const char *)(shared->pointer( fromObj ) );
					char *iSave = ((char *)(objIn->localObj ) );
					if( strncmp( sShare, iSave, fromObj->size ) )
					{
						changes = true;
						strncpy(  iSave, sShare, fromObj->size );
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name.c_str(), new COString( sShare ) );
						} else {
							((COArray * )rtn )->append(  new COString( sShare ) );
						}
					}
					sem_post( fromObj->sem );
				}
				break;
			case SL_TYPE_UNIT:
				{
					COMap *cMap = new COMap();
					for( unsigned n = 0; fromObj->nSubs > n; n++ )
					{
						checkChanges( cMap, &objIn->subs[ n ] );
					}
					if( cMap->size() )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name, cMap );
						} else {
							((COArray * )rtn )->append( cMap );
						}
					} else {
						delete cMap;
					}
				}
				break;
			case SL_TYPE_ARRAY:
				{
					COArray *cArray = new COArray();
					for( unsigned n = 0; fromObj->nSubs > n; n++ )
					{
						checkChanges( cArray, &objIn->subs[ n ] );
					}
					if( cArray->size() )
					{
						changes = true;
						if( isMap )
						{
							((COMap * )rtn )->append( fromObj->name, cArray );
						} else {
							((COArray * )rtn )->append( cArray );
						}
					} else {
						delete cArray;
					}
				}
				break;
			default:
				break;
		}
	}
	return changes;
}




LocalCppObj::LocalCppObj( SCppObj  *parent )
{
	shared = parent;

	if( parent )
	{
		unsigned sz = parent->size();
		if( sz )
		{
			/*
			 * Allocate memory for the data
			 */

			basePtr = ::operator new( sz );
			/*
			 * Copy the data
			 */
			memcpy( basePtr, parent->getBasePtr(), sz );

			/*
			 *  Recursively Build the device tree
			 */
			if( ( root = new LOCAL_CPP_OBJ) )
			{
				addSub( root, *( parent->GetBase() ) );
			}
		}
	} else {
		root = NULL;
		basePtr = NULL;

	}
}

LocalCppObj::~LocalCppObj()
{
	if( root )
	{
		for( unsigned n = 0; root->obj->nSubs > n; n++ )
		{
			deleteSub( root->subs[ n ] );
		}
		delete[] root->subs;
		delete root;
	}

	if( basePtr )
	{
		::operator delete( basePtr );
	}
}

void LocalCppObj::deleteSub( LOCAL_CPP_OBJ &lObj )
{
	for( unsigned n = 0; lObj.obj->nSubs > n; n++ )
	{
		deleteSub( lObj.subs[ n ] );
	}
	delete[] lObj.subs;
}

void LocalCppObj::addSub( LOCAL_CPP_OBJ *lObj, STRUCT_LISTS &target)
{
	if( lObj )
	{
		CppON			*hys;

		lObj->obj = &target;
		lObj->localObj = (LOCAL_CPP_OBJ *) ( (char *) basePtr + target.offset );
		lObj->subs = new LOCAL_CPP_OBJ[ target.nSubs ];

		if( target.def  && ( hys = target.def->findElement( "hysteresis" ) ) )
		{
			lObj->hysteresis = hys->toInt();
		} else {
			lObj->hysteresis = 0;
		}
		for( unsigned n = 0; target.nSubs > n; n++ )
		{
			addSub( &(lObj->subs[ n ] ), target.subs[ n ] );
		}
	}
}

