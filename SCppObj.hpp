/*
 * SCppObj.hpp
 *
 *  Created on: 30 Jun 2023 - May 26, 2024
 *      Author: jeff Hertig
 */

#ifndef SCppObj_HPP_
#define SCppObj_HPP_

#include <unistd.h>
#include "CppON.hpp"

#define SL_TYPE_NONE		0
#define	SL_TYPE_DOUBLE		1
#define SL_TYPE_INT64		2
#define SL_TYPE_INT32		3
#define SL_TYPE_INT16		4
#define	SL_TYPE_INT8		5
#define SL_TYPE_BOOL		6
#define SL_TYPE_CHAR		7
#define	SL_TYPE_UNIT		8
#define	SL_TYPE_ARRAY		9

#define SIM_WAIT_TO
typedef struct STRUCT_LISTS
{
	STRUCT_LISTS	*subs;
	char 			**names;
	sem_t			*sem;
	COMap			*def;
	std::string		name;
	uint32_t 		offset;
	uint32_t		size;
	uint32_t		time;
	uint8_t			type;
	uint8_t			nSubs;
} STRUCT_LISTS;


class SCppObj
{

public:
 										SCppObj( COMap *def, const char *segmentName = NULL, bool *initialized = NULL );
 										SCppObj( const char *configPath, const char *segmentName = NULL, bool *initialized = NULL );
 										SCppObj( const char *configPath, const char *segmentName, void(*f)( SCppObj *obj ) );
    virtual                             ~SCppObj();
    static  	CppONType             	jsonType( uint8_t typ ){ switch( typ ) { case SL_TYPE_DOUBLE:{ return DOUBLE_CPPON_OBJ_TYPE;} break; case SL_TYPE_BOOL:{ return BOOLEAN_CPPON_OBJ_TYPE;} break; case SL_TYPE_CHAR:{ return STRING_CPPON_OBJ_TYPE;} break; case SL_TYPE_UNIT:{ return MAP_CPPON_OBJ_TYPE;} break; case SL_TYPE_ARRAY:{ return ARRAY_CPPON_OBJ_TYPE; } break; case SL_TYPE_NONE: { return UNKNOWN_CPPON_OBJ_TYPE;} break; default: { return INTEGER_CPPON_OBJ_TYPE; } break; } }
    static		CppONType				jsonType( STRUCT_LISTS *lst ){ if( lst ) { return jsonType( lst->type ); } else { return UNKNOWN_CPPON_OBJ_TYPE; } }
    			CppONType				jsonType(){ return jsonType( list->type ); }
				void					setBasePointer( void *base, bool init = false, bool *initialized = NULL );
				void 					testSearchAlgorithim( );
				void					printStructList( void ){ printStructList( list, "" ); }
				void					deleteStructList( void ){ deleteStructList( list, "" ); }
				uint32_t				size(){ return list->size; }

				STRUCT_LISTS 			*getElement( const char *path, STRUCT_LISTS *base );
				bool 					waitSem( sem_t *sem )
				{
					struct 	timespec ts;
					clock_gettime( CLOCK_REALTIME, &ts);
					struct 	timespec ts2 = { ts.tv_sec, ts.tv_nsec };
					int 	s;
					if( (ts.tv_nsec += 10000000LL ) > 1000000000LL )
					{
						ts.tv_nsec -= 1000000000LL;
						ts.tv_sec++;
					}
					while( 0 != ( s = sem_trywait( sem ) ) )
					{
						usleep( 10 );
						clock_gettime( CLOCK_REALTIME, &ts2 );
						if( ( ts.tv_sec < ts2.tv_sec ) || ( ( ts.tv_sec == ts2.tv_sec ) && ( ts.tv_nsec < ts2.tv_nsec ) ) )
						{
							break;
						}
					}
					return ( 0 == s );
				}
				STRUCT_LISTS			*getElement( const char *path ) { return getElement( path, (STRUCT_LISTS *) list ); }
				STRUCT_LISTS			*GetBase(){ return list; }

				uint64_t				longValue( STRUCT_LISTS *lst, bool protect = true, bool *valid = NULL );
				uint64_t 				longValue( const char *path, bool protect= true, STRUCT_LISTS *lst = NULL, bool *valid = NULL ){ return longValue( getPointer( path, lst ), protect, valid ); }

				uint32_t 				intValue( STRUCT_LISTS *tst, bool protect = true, bool *valid = NULL );
				uint32_t 				intValue( const char *path, bool protect= true, STRUCT_LISTS *lst = NULL, bool *valid = NULL ){ return intValue( getPointer( path, lst ), protect, valid ); }

				double					doubleValue( STRUCT_LISTS *tst, bool protect = true, bool *valid = NULL );
				double	 				doubleValue( const char *path, bool protect= true, STRUCT_LISTS *lst = NULL, bool *valid = NULL ){ return doubleValue( getPointer( path, lst), protect, valid ); }

				bool					boolValue( STRUCT_LISTS *tst, bool protect = true, bool *valid = NULL );
				bool	 				boolValue( const char *path, bool protect= true, STRUCT_LISTS *lst = NULL, bool *valid = NULL ){ return boolValue( getPointer( path, lst), protect, valid ); }

				const char				*readBase64String( STRUCT_LISTS *lsg, std::string *result, bool protect = true );
				const char 				*readBase64String( const char *path, std::string *result, bool protect = true, STRUCT_LISTS *lst = NULL ){ return readBase64String( getPointer( path, lst ), result, protect ); }
				const char				*readString( STRUCT_LISTS *lsg, std::string *result, bool protect = true );
				const char 				*readString( const char *path, std::string *result, bool protect = true, STRUCT_LISTS *lst = NULL ){ return readString( getPointer( path, lst ), result, protect ); }
				char 					*readString( const char *path, char *result = NULL, size_t sz = 0, bool protect = true, STRUCT_LISTS *lst = NULL );
				double	 				*readDouble( const char *path, double *result = NULL, bool protect = true, STRUCT_LISTS *lst = NULL );
				uint64_t 				*readLong( const char *path, uint64_t *result = NULL, bool protect = true, STRUCT_LISTS *lst = NULL );
				uint32_t				*readInt( const char *path, uint32_t *result = NULL, bool protect = true, STRUCT_LISTS *lst = NULL );
				bool					*readBool( const char *path, bool *result = NULL, bool protect = true, STRUCT_LISTS *lst = NULL );
				bool					update( STRUCT_LISTS *lst, void *obj, bool protect = true );
				bool					update( const char *path, void *obj, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return update( lst, obj, protect ); }
				bool					updateString( STRUCT_LISTS *lst, const char *s, bool protect = true );
				bool					updateString( STRUCT_LISTS *lst, std::string s, bool protect = true ){ return updateString( lst, s.c_str(), protect ); }
				bool					updateString( const char *path, const char *str, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return updateString( lst, str, protect ); }
				bool					updateDouble( STRUCT_LISTS *lst, double val, bool protect = true );
				bool 					updateDouble( const char *path, double val, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return updateDouble( lst, val, protect ); }
				bool 					updateLong( STRUCT_LISTS *lst, uint64_t val, bool protect = true);
				bool 					updateLong( const char *path, uint64_t val, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return updateLong( lst, val, protect ); }
				bool 					updateInt( STRUCT_LISTS *lst, uint32_t val, bool protect = true );
				bool 					updateInt( const char *path, uint32_t val, bool protect = true, STRUCT_LISTS *lst = NULL ) { lst = getPointer( path, lst ); if( updateInt( lst, val, protect ) ) { return true; } else { fprintf( stderr, "%s[%d] Failed to find %s\n", __FILE__, __LINE__, path ); return false; } }
				bool					updateBoolean( STRUCT_LISTS *lst, bool val, bool protect = true );
				bool					updateBoolean(const char *path, bool val, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return updateBoolean( lst, val, protect ); }
				bool					updateObject( STRUCT_LISTS *lst, COMap *obj, bool protect = true );
				bool					updateObject( const char *path, COMap *obj, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return updateObject( lst, obj, protect ); }
				bool					updateArray( STRUCT_LISTS *lst, COArray *arr, bool protect = true );
				bool					updateArray( const char *path, COArray *arr, bool protect = true, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return updateArray( lst, arr, protect ); }
				bool					update( CppON *obj, STRUCT_LISTS *lst = NULL );
				bool					update( CppON *obj, const char *path, STRUCT_LISTS *lst = NULL ){ STRUCT_LISTS *tst = getPointer( path, lst ); return update( obj, tst ); }
				STRUCT_LISTS			*at( STRUCT_LISTS *lst, uint32_t idx );
				STRUCT_LISTS			*at( const char *path, uint32_t idx = 0, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return at( lst, idx ); }
				STRUCT_LISTS			*getPointer( const char *path, STRUCT_LISTS *lst = NULL ) {	if( ! lst ) { return getElement( path, (STRUCT_LISTS *) list ); } else { return getElement( path, (STRUCT_LISTS *) lst );} }
				void 					*pointer( STRUCT_LISTS *lst ){ return ( void *) ( ((uint64_t ) basePtr ) + (uint64_t )lst->offset);}
				void					*getBasePtr(){ return basePtr; }
				bool					waitSem( const char *path, STRUCT_LISTS *lst = NULL );
				bool					waitSem( STRUCT_LISTS *lst ) { if( lst ) { return( waitSem( lst->sem ) ); } return false; }
				bool					postSem( STRUCT_LISTS *lst ) { if( lst ) { sem_post( lst->sem ); return true; } return false; }
				bool					postSem( const char *path, STRUCT_LISTS *lst = NULL );
				sem_t					*openSem( int idx = -1 );
				sem_t *					getTestSem() { return (sem_t *)( (char*) basePtr + 0x20 ); }
				void					setUpdateTime( STRUCT_LISTS *lst, uint64_t t = 0 ) { if( lst ) { if( ! t ) { struct timespec tsp; clock_gettime( CLOCK_MONOTONIC, &tsp ); t = ((uint64_t) tsp.tv_sec ) * 1000LL + (uint64_t)( (( 500000 + tsp.tv_nsec ) / 1000000) ); } *((uint64_t *)((char*) basePtr + lst->time ) ) = t; } }
				void					setUpdateTime( const char *path, STRUCT_LISTS *lst = NULL, uint64_t t = 0 );
				uint64_t				getUpdateTime( STRUCT_LISTS *lst ){ if( lst ){ return *((uint64_t *)((char*) basePtr + lst->time ) ); } return 0; }
				uint64_t				getUpdateTime( const char *path, STRUCT_LISTS *lst = NULL );
				bool					getUpdateTime( struct timespec &tsp, STRUCT_LISTS *lst = NULL );
				bool					getUpdateTime( struct timespec &tsp, const char *path, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return getUpdateTime( tsp, lst ); }
				bool                    inConfig( const char *path, STRUCT_LISTS *lst = NULL ) { return ( NULL != getPointer( path, lst) ); }
				CODouble				*toCODouble( STRUCT_LISTS *val ){ if( val && SL_TYPE_DOUBLE == val->type ) { return new CODouble( *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t )val->offset) ) ); } return NULL; }
				COString				*toCOString( STRUCT_LISTS *val ){ if( val && SL_TYPE_CHAR == val->type ) { return new COString( ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t )val->offset) ) ); } return NULL; }
				COBoolean				*toCOBoolean( STRUCT_LISTS *val ){ if( val && SL_TYPE_BOOL == val->type ) { return new COBoolean( *( ( bool *) ( ( (uint64_t ) basePtr ) + (uint64_t )val->offset) ) ); } return NULL; }
				COInteger				*toJInt64( STRUCT_LISTS *val ){ if( val && SL_TYPE_INT64 == val->type ) { return  new COInteger( (uint64_t) *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ); } return NULL; }
				COInteger				*toJInt32( STRUCT_LISTS *val ){ if( val && SL_TYPE_INT32 == val->type ) { return  new COInteger( (int) *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ); } return NULL; }
				COInteger				*toJInt16( STRUCT_LISTS *val ){ if( val && SL_TYPE_INT16 == val->type ) { return  new COInteger( (int) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ); } return NULL; }
				COInteger				*toJInt8( STRUCT_LISTS *val ){ if( val && SL_TYPE_INT8 == val->type ) { return  new COInteger( (int) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ); } return NULL; }
				COInteger				*toJInterger( STRUCT_LISTS *val ) ;
				COArray					*toCOArray( STRUCT_LISTS *root = NULL );
				COArray					*toCOArray( const char *path, STRUCT_LISTS *root = NULL ){ root = getPointer( path, root ); return toCOArray( root ); }
				COMap					*toCOMap( STRUCT_LISTS *root = NULL );
				COMap					*toCOMap( const char *path, STRUCT_LISTS *root = NULL ){ root = getPointer( path, root ); return toCOMap( root ); }
				CppON					*toCppON( STRUCT_LISTS *root = NULL );
				CppON					*toCppON( const char *path, STRUCT_LISTS *lst = NULL ){  lst = getPointer( path, lst ); return toCppON( lst ); }
				uint64_t				toLong( STRUCT_LISTS *val );
				uint64_t				toLong( const char *path, STRUCT_LISTS *val = NULL ){  val = getPointer( path, val ); return toLong( val ); }
				uint32_t				Int( STRUCT_LISTS *val );
				uint32_t				Int( const char *path, STRUCT_LISTS *val = NULL ){  val = getPointer( path, val ); return Int( val ); }
				double					toDouble( STRUCT_LISTS *val );
				double					toDouble( const char *path, STRUCT_LISTS *val = NULL ){  val = getPointer( path, val ); return toDouble( val ); }
				bool					toBoolean( STRUCT_LISTS *val );
				bool					toBoolean( const char *path, STRUCT_LISTS *val = NULL ){  val = getPointer( path, val ); return toBoolean( val ); }
				char 					*String( STRUCT_LISTS *val, char *var = NULL, int sz = 0 );
				char 					*String( const char *path, STRUCT_LISTS *val = NULL, char *var = NULL, int sz = 0 ){  val = getPointer( path, val ); return String( val, var, sz ); }

				bool 					syncInt( CppON *obj, STRUCT_LISTS *lst );
				bool 					syncDouble( CppON *obj, STRUCT_LISTS *lst );
				bool 					syncString( CppON *obj, STRUCT_LISTS *lst );
				bool 					syncBoolean( CppON *obj, STRUCT_LISTS *lst );
				bool 					syncMap( CppON *obj, STRUCT_LISTS *lst );
				bool 					syncArray( CppON *obj, STRUCT_LISTS *lst );
				bool					sync( CppON *obj, STRUCT_LISTS *root );
				bool					sync( CppON *obj, const char *path, STRUCT_LISTS *root = NULL ){ STRUCT_LISTS *tst = getPointer( path, root ); return sync( obj, tst ); }
				bool					waitForUpdate( STRUCT_LISTS *lst, uint64_t start = 0, uint64_t to = 0 );
				bool 					waitForUpdate( const char *path,  STRUCT_LISTS *lst = NULL, uint64_t start = 0, uint64_t to = 0 ) { STRUCT_LISTS *tst = getPointer( path, lst ); return waitForUpdate( tst, start, to ); }
		static	bool					isInteger( STRUCT_LISTS *val ) { return ( val && (SL_TYPE_INT64 == val->type || SL_TYPE_INT32 == val->type || SL_TYPE_INT16 == val->type || SL_TYPE_INT8 == val->type ) ); }
		static	bool					isDouble( STRUCT_LISTS *val ) { return ( val && SL_TYPE_DOUBLE == val->type ); }
		static	bool					isBoolean( STRUCT_LISTS *val ) { return ( val && SL_TYPE_BOOL == val->type ); }
		static	bool					IsNumber( STRUCT_LISTS *val ) { return ( val && ( isDouble( val )  || isInteger( val ) || isBoolean( val ) ) ); }
		static	bool					isMap( STRUCT_LISTS *val ) { return ( val && SL_TYPE_DOUBLE == val->type ); }
		static	bool					IsArray( STRUCT_LISTS *val ) { return ( val && SL_TYPE_ARRAY == val->type ); }
		static	bool					isString( STRUCT_LISTS *val ) { return ( val && SL_TYPE_CHAR == val->type ); }
		static	bool					isObj( STRUCT_LISTS *val ){ return ( val && SL_TYPE_DOUBLE <= val->type && SL_TYPE_ARRAY >= val->type ); }

				bool					equals( CppON &obj, STRUCT_LISTS *val  );
				bool					equals( CppON &obj, const char *path, STRUCT_LISTS *lst = NULL ){ lst = getPointer( path, lst ); return equals( obj, lst ); }
				COMap					*getConfig(){ return config; }
private:
				void					initializeObject( const char *segmentName, bool *initialized );
				void					printStructList( STRUCT_LISTS *lst, std::string indent );
				void					deleteStructList( STRUCT_LISTS *lst, std::string indent );

				void					arrayDefaults( COMap *def, STRUCT_LISTS *lst, std::string indent, const char *name, sem_t *sem );
				void 					unitDefaults( COMap *def, STRUCT_LISTS *lst, std::string indent, const char *name, sem_t *sem );
				void 					listArraySems( COMap *def, STRUCT_LISTS *lst, sem_t *sem );
				void 					listSems( COMap *def, STRUCT_LISTS *lst, sem_t *sem );
				uint32_t 				buildArray( COMap *def, STRUCT_LISTS *unit, std::string indent, const char *name );
				uint32_t 				buildUnit( COMap *def, STRUCT_LISTS *unit, std::string indent, const char *name );
				void					buildArrayNames( COMap *def, std::string indent, char **out[] );
				void 					buildNames( COMap *def,std::string indent, char **out[]  );
				void 					doTest( const char *path );

				std::string				sharedSegmentName;
				std::vector<sem_t *>	sems;
				void					*basePtr;
				COMap					*config;
				STRUCT_LISTS			*list;
				uint32_t				timeOffset = 0;
				int						doubleOffset = 0;
				int 					int64Offset = 0;
				int 					int32Offset = 0;
				int						int16Offset = 0;
				int 					eightBitOffset = 0;
				int						charOffset = 0;
				bool					sharedMemoryAllocated = false;
};

#endif /* SCppObj_HPP_ */
