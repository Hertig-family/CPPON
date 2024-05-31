/*
 * SCppObj.cpp
 *
 *  Created on: 30 Jun 2023 - May 26, 2024
 *      Author: jeff Hertig
 *
 *  Description:
 *      The SCppObj.cpp class is an extension to the CppON class in that it requires the class in its initialization and to function.
 *
 *      Its purpose is to create and maintain a shared memory object that can be used by multiple applications and persist over shutdowns and startups on those applications.
 *      The idea is that the object data structure is defined by a JSON file, and behaves much like an instance of a CPPON hierarchy, referenced by a given name and can be created by any application.
 *
 *      There are a couple of features that are important to understand with the SCppObj.  The first is that like a JSON object (or CppON) all data is referenced by string path names where
 *      the data is referenced like in JSON as objects, arrays, strings, doubles, integers (Long of int) or bool (boolean) but unlike a CppON the data is not stored in ASCII or in actual
 *      structures of objects. Also, once created the structure is fixed. Space is allocated in shared memory based on information in the description. Strings have a maximum length.
 *      Path names can be delimited by either a dot "." or forward slash "/".  There is no difference.
 *
 *		The JSON description file:
 *
 *      When instantiated the first parameter of the SCppObj constructor is either a path name to a JSON file or a CppON::COMap.  This JSON object is a description file that defines the layout
 *      of the SCppObj that is created.  It is organized as a COMap object containing other COMap objects which represent JSON objects.  Each object describes either a data type which could be a
 *      container object or one of the basic types: string, float, bool, or int.  The data type is defined by a string value (which is required) called "type".  For a object type the type value is
 *      set to "unit", and will be implemented as a "COMap" containing other objects.  Another required value for strings and integers is the "size" integer value. This give the number of bytes
 *      allocated for the data. (It's not required for booleans, nulls, or floats because the space for those is fixed. For strings the size it the maximum number of characters allocated plus one
 *      for the null character.  For the integer types it must be 8, 4, 2 or 1.
 *
 *      For all basic types the "defaultValue" must be defined which is the initial value of the field on creation.  And, for the "float" type a "precision" value can be defined which indicates the
 *      number of significant digits when converting it to a string.
 *
 *      To summarize: The object is defined by a structure of structures with each structure representing a JSON value.  The value can be a type "COMap" which holds other JSON data and is denoted by
 *      the "type": "unit".  Or, it can be a basic type and denoted by the "type": "string" | "float" | "int" | "bool".  "int" and "string" types must have a "size" parameter and "floats" can have a
 *      "precision" value.  All basic values must have a defaultValue specified which the object is set to in initialization.
 *
 *      The SCppObj:
 *
 *      Besides being stored in system shared memory, the SCppObj is far more efficient then storing data in either a CppON or JSON strings.  This is because the offset to the data is fixed and known.
 *      However this comes at a cost:  Once created, it's structure if fixed and cannot be changed.  Strings have a fixed maximum length.  Of course another great thing about it is the data is stored
 *      in system shared memory and is only be deleted by a reboot or performing the shared memory library call: shm_unlink( sm_name );  Where sm_name is the name of the SCppObj. (But the system will
 *      only delete the shared memory segment when all references to it are closed.)  All applications on the system can access it without fear of thread collisions or corruption.  This is due to its
 *      use of system semaphores.  These are handled by the class, but when desired to maintain synchronization of data can be handled by the application.
 *
 *      It's important to note that the data is not stored in structures nor is accessible without accessing it through path names. When an application first instantiates the SCppObj object it builds
 *      an internal network of pointers called "STRUCT_LISTS" which are references to all objects and where they are stored in memory.  These Object contain type information and pointers to all
 *      contents and data that the variable accesses.  You can get a reference to one of these objects with the "getElement" request but should NEVER directly modify the contents one of these objects.
 *
 *		Each data type and integer sizes have their own "accessors" to get and update data in the structure.  There are three types of accessors,  An example of each is:
 *			*  bool updateString( const char *path, const char *str, bool protect = true, STRUCT_LISTS *lst = NULL ); ( returns true if value is valid )
 *			*  double *readDouble( const char *path, double *result = NULL, bool protect = true, STRUCT_LISTS *lst = NULL );
 *			*  double Doublevalue( const char *path, bool protect= true, STRUCT_LISTS *lst = NULL, bool *valid = NULL )
 *
 *		With all data access calls the first parameter in the call defines the element the operation is to be performed on.  This can be a "STRUCT_LISTS" pointer to the element (not usually used)
 *		or a path name to the element. (To avoid lengthy searches it is best to use relative addressing of data.)  The path name starts at the top level and transitions through the object tree based
 *		on a dot (.) or slash (/) delimited string.  In the case of a "update" or "read" accessor call, the second parameter is always the value or pointer to the value to be returned. "Value"
 *		accessors don't have this parameter as the value is returned from the method.
 *
 *		For character array only, "read" calls the next parameter is the max size of the space to store the data.
 *
 *		The next parameter is a boolean value where: true means to use (checkout) the semaphore for the structure and false means to ignore the semaphore for this operation. (Make sure that if you enter
 *		true, the default, that you haven't already obtained the semaphore.  Remember: there is just one semaphore for all the data in the same structure.) Its a best practice to never "hold" more then
 *		one semaphore at a time and to only do so for a minimum amount of time. Making sure that you allways return it.
 *
 *		The next optional parameter can be a "Parent" STRUCT_LISTS * which the first parameter is relative to.  The use of this can be a big performance boost as finding the data pointer is much
 *		faster.  (The default is NULL, and the "path" must be relative to the root)
 *
 *		On "value" accessors there is another optional parameter which is a point to a boolean value.  If the pointer is non-NULL (default is NULL) the return value will be true if the value is found,
 *		meaning the returned value is valid.
 *
 *
 *      There are three possible constructors
 *
 *      	1) SCppObj::SCppObj( COMap *def, const char *segmentName, bool *initialized );
 *      	2) SCppObj::SCppObj( const char *configPath, const char *segmentName, bool *initialized )
 *      	3) SCppObj::SCppObj( const char *configPath, const char *segmentName, void(*f)( SCppObj *obj ) )
 *
 *    	The first two are the same, differing only in the fact that the fist is given a CppON::COMap for its configuration while the second is given a pathname of a JSON file that contains
 *    	the configuration.   The second parameter is the segment name which will identify the shared memory segment and the third optional parameter is a pointer to a boolean flag so the
 *    	calling application can determine if the instantiation of the object was the one that created and initialized it, in case more needs to be done.
 *
 *    	The third constructor allows you to pass a callback routine in the form of "void p( SCppObj *object );" which will be called if the object is first created to allow a program to do
 *    	further initialization that isn't contained in the JSON file.  (NOTE: as there usually isn't any guarantee which application first instantiates thus creating the object, all
 *    	application should use this form if it is required that the data altered by the initiation callback.
 *
 *    	This object can produce very complex multi-level data structures in a very optimized storage space.  It protects access of each structure buy use of semaphores, one for each structure.
 *    	Semaphores are used by default for all accesses, both read or write.  However this functionality can be overridden on a call by call basis.  Also, for performance reasons you can "capture"
 *    	the semaphore for a structure, performs multiple access to data in the structure without using them and then release it.
 *
 *    	Another big performance benefit when accessing data in a single structure is that a reference to the structure itself can be retrieved and than accesses to data in it can be done relative
 *    	to it.  I.E  Suppose you have a structure that is multiple levels deep in the configuration like system/configuration/tsp which contain the configuration information for the tracking signal
 *    	processor.  With a SCppObj called "sysdata" you can do something like the following:
 *    			uint_64		hardwareRev = 0;
 *    			int_32		softwareRev = 0;
 *    			std::string	address("");
 *    			char 		amazonId[ 18 ];
 *    			bool		hasTSP = false;
 *    			bool		valid = false;
 *
 *    	        STRUCT_LISTS *tsp = sysdata->getElement( "configuration.tsp" );					// Get a reference object to the tsp section of the configuration data. (Note: Could of used
 *    	        																				// "configuration/tsp" )
 *    	        if( tsp )																		// Make sure your request was successful. ( tsp is a reverence don't delete it. )
 *    	        {
 *					sysdata->waitSem( "address", tsp );											// Optional: Capture the semaphore for the whole tsp section so values can be changed in a group
 *																								// and increase performance
 *
 *    	        	sysdata->readString( "address", &address, false, tsp );						// read a string value "configuration.tsp.address" into a std::string value called "address" without
 *    	        																				// getting the semaphore and using the tsp reference
 *    	        	sysdata->readString( "amazon_number, amazonId, 18, false, tsp );			// read a string value into a character array of size 18 (size must include null character)
 *    	        	sysdata->readBool( "installed", &hasTSP, false, tsp );						// read a boolean value called installed into a value hasTSP
 *					sysdata->readInt( "software_rev", &softwareRev, false, tsp );				// read an 32 bit integer called software_rev
 *					hardwareRev = sysdata->longValue( "hardware_rev", false, tsp, &valid );  	// Another way to read a value.  This time returning the value.  The optional parameter "valid" is set
 *																								// to true if the return value is valid. I.E. the "configuration.tsp.hardware_rev" 8 byte integer exists.
 *					sysdata->updateString( "user", "Peter", false, tsp );						// modify the string value "configuration/tsp/user" to make it contain "Peter\0"
 *					address = "192.168.123.156";
 *					sysdata->updateString( "address", address, false, tsp);						// modify the string value "configuration/tsp/address" to make it contain "192.168.123.156\0"
 *
 *					sysdata->postSem( "address, tsp );											// Free the semaphore so other threads can access the data.
 *				}
 *
 *		Like with the CppON there are calls to determine the type like isInteger, isDouble, isString ....
 *
 *		There are also a whole bunch of calls to return a CppON value of the various types (To prevent memory leaks these should be deleted.)   They are things like:
 *			* toCODouble() returns a CODouble
 *			* toCOInteger() return a COInteger
 *			* toCOMap() returns a COMap of a structure
 *			* ToJObject() returns an object of whatever type it is.
 *			* and so forth
 *
 *		There are other functions which you can look at the header file for but this pretty much covers what you need.
 *
 *		Semaphore use:
 *		Semaphores are a necessary evil of multi-threaded programming.  The protect data from corruption and reading invalid or unsynchronized data.  Especially strings.  However they are extremely
 *		dangerous if misused because the can cause threads to become permanently locked.  Although in its basic form the CObj can be used without every dealing with them, in which case it will handle
 *		them safely without intervention.  Or access can be used in such a way as to ignore the semaphores on a call by call basis.  Considerable performance and safety can be gained by a little
 *		management on the part of the programmer.  To avoid problems there are a few basic rules that must always be followed when dealing with them:
 *			1) Never own two semaphores at the same time.  This is just looking for trouble  especially when one thread can have one and then try to check out another while another thread has the second
 *			   on and is try to check out the first.
 *			2) Be very careful never to try to check out the same semaphore twice. This is a guaranteed thread lock.
 *			3) When you check out a semaphore, get done quickly and free it up as soon as possible.  Avoid calling subroutines or methods while you own a semaphore.
 *			4) Don't checkout semaphores from within an interrupt.
 *			5) Be very careful of multiple paths or exits from code after checking out a semaphore which could cause the code to not free it.
 *
 *		Best practices:
 *		* The shared memory object can be very large and complicated but usually a section of code will only be working within a certain area.  In which case it is always best to get a reference to
 *		  the "piece" of the object and access the information relative to it.  You can do this with or without owning the semaphore to the structure.
 *		* If you know that your thread is the only one modifying a section of data, you can bypass the semaphore checking on reads.
 *
 *		EDIT: 11/21/2023
 *		We added support for arrays in the CObj library.
 *		To define an array, it's still done with Objects the same way as any object is defined with some restrictions:
 *			1) the type must be "array"
 *			2) All elements MUST be named sequential number starting with "0", "1" , "2", "3".....
 *			3) Use the "at() method to get the STRUCT_LIST * of an element in the array
 *			4) You can use the index in a search string like "configuration/C2/data/4" to get the forth element of the array data in the C2 object of the Configuration
 *			4) In fact you can use can use the index in any search string. I.E. globalObj->doubleValue( "configuration/C2/data/2/second") => The second element of the data array is an object with the value "second"
 *			   which in this case is a type double.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string>
#include <vector>
#include <csignal>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <dirent.h>


#include "CppON.hpp"

using namespace std;

#include "SCppObj.hpp"

const char *JCOB_TYPES[] = { "float", "int", "bool", "string", "unit", "array" };
#define SCppObj_FLOAT "float"
#define SCppObj_INT "int"
#define SCppObj_BOOL "bool"
#define SCppObj_STRING "string"
#define SCppObj_UNIT "unit"
#define SCppObj_ARRAY "array"

/*******************************************************************************************/
/*                                                                                         */
/*                                 SCppObj                                                   */
/*                                                                                         */
/*******************************************************************************************/

void SCppObj::initializeObject( const char *segmentName, bool *initialized )
{
	unsigned 		units = 0;
	unsigned		arrays = 0;
	int				sz = 1;


    basePtr = NULL;
    timeOffset = 0x20;
    doubleOffset = 0;
    int64Offset = 0;
    int32Offset = 0;
    int16Offset = 0;
    eightBitOffset = 0;
    charOffset = 0;
    sharedMemoryAllocated = false;

	list = new STRUCT_LISTS;

	std::string ident("\t" );

	/*
	 * Figure out how many units the base has in it then use that to allocate the structure
	 */
	for( std::map<std::string, CppON *>::iterator it = config->begin(); config->end() != it; it++ )
	{
		if( CppON::isMap( it->second ) && it->first.compare( "update" ) )
		{
			sz++;
			units++;
		} else if( CppON::isArray( it->second ) && it->first.compare( "update" ) ) {
			sz++;
			arrays++;
		}
	}

	list->subs = new STRUCT_LISTS[ list->nSubs = ( units + arrays) ];
	list->offset = timeOffset;
	list->type = SL_TYPE_UNIT;
	list->name = "base";
	list->size = 0;
	list->time = 0;
	list->names = NULL;
	list->def = config;

	buildNames( config, "", &( list->names )  );

	/*
	 * Now go through and add all the units
	 */
	for( units = 0; list->names[ 2 * units ]; units++ )
	{
		CppON		*oPtr;
		COString 	*sPtr;
		const char 	*c = list->names[ (2 * units ) + 1 ];

		COMap 		*mp = (COMap *) config->findElement( c );

		if( CppON::isMap ( mp )  )
		{
			std::string typ = SCppObj_UNIT;

			STRUCT_LISTS *ls = &( (STRUCT_LISTS *) list->subs )[ units ];
			ls->names = NULL;
			ls->nSubs = 0;
			ls->def = mp;

			if( CppON::isString( sPtr = (COString *) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( ! strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
				ls->type = SL_TYPE_UNIT;
				ls->offset = 0;
				ls->time = 0;
				list->size += buildUnit( (COMap *) mp, ls, ident, c );
			} else if( ! strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
				ls->type = SL_TYPE_ARRAY;
				ls->offset = 0;
				ls->time = 0;
				list->size += buildArray( (COMap *) mp, ls, ident, c );
			} else {

				ls->name = c;
				ls->time = timeOffset;
				timeOffset += sizeof( uint64_t );

				if( !strcasecmp( typ.c_str(), SCppObj_INT ) ) {
					int sz = 4;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					switch( sz )
					{
						case 1:
							ls->type = SL_TYPE_INT8;
							ls->offset = (uint32_t) eightBitOffset;
							ls->size = sizeof( uint8_t );
							eightBitOffset += ls->size;
							break;
						case 2:
							ls->type = SL_TYPE_INT16;
							ls->size = sizeof( uint16_t );
							ls->offset = (uint32_t) int16Offset;
							int16Offset += ls->size;
							break;
						case 8:
							ls->type = SL_TYPE_INT64;
							ls->size = sizeof( uint64_t );
							ls->offset = (uint32_t) int64Offset;
							int64Offset += ls->size;
							break;
						default:
							ls->type = SL_TYPE_INT32;
							ls->size = sizeof( uint32_t );
							ls->offset = (uint32_t) int32Offset;
							int32Offset += ls->size;
							break;
					}
				} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
					ls->type = SL_TYPE_DOUBLE;
					ls->size = sizeof( double );
					ls->offset = (uint32_t) doubleOffset;
					doubleOffset += ls->size;
				} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
					int sz = 16;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					ls->size = sz;
					ls->type = SL_TYPE_CHAR;
					charOffset += sz;
					ls->offset = (uint32_t) charOffset;
				} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
					ls->type = SL_TYPE_BOOL;
					ls->size = 1;
					ls->offset = (uint32_t) eightBitOffset;
					eightBitOffset++;
				}
			}
		}
	}

	uint32_t doubleStart = timeOffset;
	uint32_t int64Start = doubleStart + doubleOffset;
	uint32_t int32Start = int64Start + int64Offset;
	uint32_t int16Start = int32Start + int32Offset;
	uint32_t eightBitStart = int16Start + int16Offset;
	uint32_t charStart = eightBitStart + eightBitOffset;
	list->size = charStart + charOffset;

	timeOffset = 0x20;
	doubleOffset = doubleStart;
	int64Offset = int64Start;
	int32Offset = int32Start;
	int16Offset = int16Start;
	eightBitOffset = eightBitStart;
	charOffset = charStart;

	/*
	 * If we passed it a segmentName we want it to create a shared memory segment and store it in it
	 */
	if( segmentName )
	{
	    int 			sm_fd = -1;
	    struct stat 	_stat;
    	uint32_t 		sz = list->size;
    	void			*ptr;

        if( 0 > sm_fd )
        {
            if( 0 >= (sm_fd = shm_open( segmentName, O_CREAT | O_RDWR, 0666 ) ) )
            {
            	fprintf( stderr, "%s[%d] Failed to open shared memory segment: %s\n",__FILE__, __LINE__, segmentName );
                return;
            }
        }

        if( 0 != fstat( sm_fd, &_stat ) || sz != (size_t) _stat.st_size )
        {
            if( ftruncate( sm_fd, sz ) )
            {
            	fprintf( stderr, "%s[%d] Failed to set shared Memory size to %d\n", __FILE__, __LINE__, sz );
            	return;
            }
        }
        if( NULL == ( ptr = ( (char *) mmap( 0, sz, PROT_WRITE, MAP_SHARED, sm_fd, 0 ) ) ) )
        {
        	fprintf( stderr, "%s[%d] Failed to map shared memory: %d - %s\n",__FILE__,__LINE__, errno, strerror( errno ) );
        	return;
        }
        sharedMemoryAllocated = true;
        sharedSegmentName = segmentName;

        setBasePointer( ptr, true, initialized );

        const char              *smPath = "/dev/shm";
        char					smName[ 64 ];
        DIR                     *dir;
        struct dirent			*ent;

        strcpy( smName, "/dev/shm/" );
        if( ( dir = opendir( smPath ) ) )
        {
            while( ( ent = readdir( dir ) ) )
            {
                if( '.' != ent->d_name[ 0 ]  )
                {
                	strncpy( &smName[ 9 ], ent->d_name, 54 );
                	if( ! stat( smName, & _stat ) && !( _stat.st_mode & 2 ) )
                	{
                		if( 0 > chmod( smName, 0666) )
                		{
	    	    			fprintf( stderr, "%s[%d] Failed to change permissions of %s to oscp: %d - %s\n",__FILE__,__LINE__, smName, errno, strerror( errno ) );
                		}
                	}
                }
            }
            closedir( dir );
	   	}
	}
}

SCppObj::SCppObj( COMap *def, const char *segmentName, bool *initialized )
{
	if( ! CppON::isMap( def ) )
	{
		throw std::invalid_argument( "Not given a valid JSON Object" );
	}
	/*
	 * config is the  object that contains the structure configuration
	 */
	config = new COMap( def );
	initializeObject( segmentName, initialized );
}

SCppObj::SCppObj( const char *configPath, const char *segmentName, bool *initialized )
{
	if( !( CppON::isMap( config = (COMap *) CppON::parseJsonFile( configPath ) ) ) )
	{
		char buf[ 128 ];
		snprintf( buf, 127, "Failed read %s file or it is not a valid JSON Object", configPath );
		throw std::invalid_argument( buf );
	}
	initializeObject( segmentName, initialized );
}
SCppObj::SCppObj( const char *configPath, const char *segmentName, void(*f)( SCppObj *obj ) )
{
	bool initialized = false;

	if( !( CppON::isMap( config = (COMap *) CppON::parseJsonFile( configPath ) ) ) )
	{
		char buf[ 128 ];
		snprintf( buf, 127, "Failed read %s file or it is not a valid JSON Object", configPath );
		throw std::invalid_argument( buf );
	}
	initializeObject( segmentName, &initialized );
	if( initialized && f )
	{
		f( this );
	}
}


SCppObj::~SCppObj()
{
	uint32_t	sz = list->size;
	delete config;
	deleteStructList( list, "" );
	delete list;
	for( std::vector<sem_t *>::iterator it = sems.begin(); sems.end() != it; it++ )
	{
		sem_close( *it );
	}
	sems.clear();
	if( sharedMemoryAllocated )
	{
        munmap( basePtr, sz );
	}
}


void SCppObj::doTest( const char *path )
{
	 STRUCT_LISTS *tst = getElement( path, (STRUCT_LISTS *) list );
	 if( tst )
	 {
		 fprintf( stderr, "name: %s, type: %d,", path, tst->type );
		 void * addr = (void *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset );
		 fprintf( stderr, ", offset: 0x%X", tst->offset );

		 switch ( tst->type )
		 {
		 	 case SL_TYPE_DOUBLE:
		 		 fprintf( stderr, ", default: %lf\n", *((double*) addr) );
		 		 break;
		 	 case SL_TYPE_INT64:
#if SIXTY_FOUR_BIT
		 		 fprintf( stderr, ", default: 0x%lX\n", *((uint64_t *) addr ) );
#else
		 		 fprintf( stderr, ", default: 0x%llX\n", *((uint64_t *) addr ) );
#endif
		 		 break;
		 	 case SL_TYPE_INT32:
		 		 fprintf( stderr, ", default: 0x%X\n", *((uint32_t *) addr ) );
		 		 break;
		 	 case SL_TYPE_INT16:
		 		 fprintf( stderr, ", default: 0x%X\n", (unsigned) *((uint16_t *) addr ) );
		 		 break;
		 	 case SL_TYPE_INT8:
		 		 fprintf( stderr, ", default: 0x%X\n", (unsigned) *((uint8_t *) addr ) );
		 		 break;
		 	 case SL_TYPE_BOOL:
		 		 fprintf( stderr, ", default: %s\n", ( 0 != *((uint8_t *) addr ) )? "True":"False" );
		 		 break;
		 	 case SL_TYPE_CHAR:
		 		 fprintf( stderr, ", default: %s\n", ((char *) addr ) );
		 		 break;
		 	 default:
		 		 break;
		 }
	 } else {
		 fprintf( stderr, "name: %s, NOT FOUND!!!!\n", path );
	 }
}

void SCppObj::testSearchAlgorithim(  )
{
	doTest( "configuration/az_drive/software_rev" );
	doTest( "configuration/feed_2/has_lock_status");
	doTest( "positioner_status");
	doTest( "positioner_status/motor_bus");
	doTest( "positioner_status/acu_status" );
	doTest( "axis_status/azt/position" );
	doTest( "environmental_status/tilt_encoder/temperature" );
	doTest( "two_line_elements/satellite" );
	doTest( "two_line_elements/line_1" );
	doTest( "two_line_elements/line_2" );

}

COInteger *SCppObj::toJInterger( STRUCT_LISTS *val )
{
	COInteger *iv = NULL;
	if( val )
		switch( val->type )
		{
			case SL_TYPE_INT64:
				iv = toJInt64( val );
				break;
			case SL_TYPE_INT32:
				iv = toJInt32( val );
				break;
			case SL_TYPE_INT16:
				iv = toJInt16( val );
				break;
			case SL_TYPE_INT8:
				iv = toJInt8( val );
				break;
			default:
				break;
		}
	return iv;
}
COArray *SCppObj::toCOArray( STRUCT_LISTS *root )
{
	COArray* rtn = NULL;
	if( root &&  SL_TYPE_ARRAY == root->type )
	{
		rtn = new COArray();
		for( unsigned i = 0; root->nSubs > i; i++ )
		{
			STRUCT_LISTS &s = ((STRUCT_LISTS *) root->subs)[ i ];
			switch( s.type )
			{
				case SL_TYPE_DOUBLE:
					rtn->append( toCODouble( &s ) );
					break;
				case SL_TYPE_INT64:
					rtn->append( toJInt64( &s ) );
					break;
				case SL_TYPE_INT32:
					rtn->append( toJInt32( &s ) );
					break;
				case SL_TYPE_INT16:
					rtn->append( toJInt16( &s ) );
					break;
				case SL_TYPE_INT8:
					rtn->append( toJInt8( &s ) );
					break;
				case SL_TYPE_BOOL:
					rtn->append( toCOBoolean( &s ) );
					break;
				case SL_TYPE_CHAR:
					rtn->append( toCOString( &s ) );
					break;
				case SL_TYPE_UNIT:
					rtn->append( toCOMap( &s ) );
					break;
				case SL_TYPE_ARRAY:
					rtn->append( toCOArray( &s ) );
					break;
				default:
					break;
			}
		}
	}

	return rtn;
}
COMap *SCppObj::toCOMap( STRUCT_LISTS *root )
{
	COMap *rtn = NULL;
	if( root &&  SL_TYPE_UNIT == root->type )
	{
		rtn = new COMap();
		for( unsigned i = 0; root->nSubs > i; i++ )
		{
			STRUCT_LISTS &s = ((STRUCT_LISTS *) root->subs)[ i ];
			const char *name = s.name.c_str();
			switch( s.type )
			{
				case SL_TYPE_DOUBLE:
					rtn->append( name, toCODouble( &s ) );
					break;
				case SL_TYPE_INT64:
					rtn->append( name,  toJInt64( &s ) );
					break;
				case SL_TYPE_INT32:
					rtn->append( name,  toJInt32( &s ) );
					break;
				case SL_TYPE_INT16:
					rtn->append( name,  toJInt16( &s ) );
					break;
				case SL_TYPE_INT8:
					rtn->append( name,  toJInt8( &s ) );
					break;
				case SL_TYPE_BOOL:
					rtn->append( name,  toCOBoolean( &s ) );
					break;
				case SL_TYPE_CHAR:
					rtn->append( name,  toCOString( &s ) );
					break;
				case SL_TYPE_UNIT:
					rtn->append( name,  toCOMap( &s ) );
					break;
				case SL_TYPE_ARRAY:
					rtn->append( name,  toCOArray( &s ) );
					break;
				default:
					break;
			}
		}
	}
	return rtn;
}

CppON *SCppObj::toCppON( STRUCT_LISTS *root )
{
	uint8_t typ = 0;
	if( ! root )
	{
		root = list;
	}
	if( (typ = root->type) >= SL_TYPE_DOUBLE && typ <= SL_TYPE_ARRAY )
	{
		switch( typ )
		{
		case SL_TYPE_DOUBLE:
			return toCODouble( root );
			break;
		case SL_TYPE_INT64:
			return toJInt64( root );
			break;
		case SL_TYPE_INT32:
			return toJInt32( root );
			break;
		case SL_TYPE_INT16:
			return toJInt16( root );
			break;
		case SL_TYPE_INT8:
			return toJInt8( root );
			break;
		case SL_TYPE_BOOL:
			return toCOBoolean( root );
			break;
		case SL_TYPE_CHAR:
			return toCOString( root );
			break;
		case SL_TYPE_UNIT:
			return toCOMap( root );
			break;
		case SL_TYPE_ARRAY:
			return toCOArray( root );
			break;
		default:
			break;
		}
	}
	return NULL;
}
bool SCppObj::syncInt( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;

	if( lst && CppON::isInteger( obj ) )
	{
		switch( lst->type )
		{
			case SL_TYPE_INT64:
				{
					long long l = (long long ) *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) );
					if( (result = ( l != ((COInteger*)obj)->toLongInt() ) ) )
					{
						*((COInteger*) obj ) =  l;
					}
				}
				break;
			case SL_TYPE_INT32:
				{
					int l = ( int ) *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) );
					if( (result = ( l != ((COInteger*)obj)->toInt() ) ) )
					{
						*((COInteger*) obj ) =  l;
					}
				}
				break;
			case SL_TYPE_INT16:
				{

					int l = ( int ) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) );
					if( (result = ( l != ((COInteger*)obj)->toInt() ) ) )
					{
						*((COInteger*) obj ) =  l;
					}
				}
				break;
			case SL_TYPE_INT8:
				{
					int l = ( int ) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) );
					if( (result = ( l != ((COInteger*)obj)->toInt() ) ) )
					{
						*((COInteger*) obj ) =  l;
					}
				}
				break;
			default:
				break;
		}
	}
	return result;
}
bool SCppObj::syncDouble( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;
	if( lst && SL_TYPE_DOUBLE == lst->type && CppON::isDouble( obj ) )
	{
		double d = *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) );
		if( (result = ( d != ((CODouble *)obj)->toDouble() ) ) )
		{
			*((CODouble *) obj ) =  d;
		}
	}
	return result;
}
bool SCppObj::syncString( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;
	if( lst && SL_TYPE_CHAR == lst->type &&  CppON::isString( obj ) )
	{
		char *s = ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) );
		if( (result = ( 0 != strcmp( s, ((COString *)obj)->c_str() ) ) ) )
		{
			*((COString *) obj ) =  s;
		}
	}
	return result;
}
bool SCppObj::syncBoolean( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;
	if( lst && SL_TYPE_BOOL == lst->type &&  CppON::isDouble( obj ) )
	{
		bool v = ( 0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
		if( (result = ( v  != ((COBoolean *)obj)->value() ) ) )
		{
			*((COBoolean *) obj ) =  v;
		}
	}
	return result;
}
bool SCppObj::syncMap( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;

	if( lst &&  SL_TYPE_UNIT == lst->type && CppON::isMap( obj ) )
	{
		for( std::map<std::string, CppON *>::iterator it = ((COMap *) obj)->begin(); ((COMap *) obj)->end() != it; it++ )
		{
			const char 	*name = it->first.c_str();
			CppON		*jobj = it->second;
			for( unsigned i = 0; lst->names[ i ]; ++i )
			{
				if( !strcmp( name, lst->names[ ++i ] ) )
				{
					STRUCT_LISTS *l = &((STRUCT_LISTS *) lst->subs)[ i / 2 ];
					switch( jobj->type() )
					{
						case INTEGER_CPPON_OBJ_TYPE:
							if( syncInt( jobj, l ) )
							{
								result = true;
							}
							break;
						case DOUBLE_CPPON_OBJ_TYPE:
							if( syncDouble( jobj, l ) )
							{
								result = true;
							}
							break;
						case STRING_CPPON_OBJ_TYPE:
							if( syncString( jobj, l ) )
							{
								result = true;
							}
							break;
						case BOOLEAN_CPPON_OBJ_TYPE:
							if( syncBoolean( jobj, l ) )
							{
								result = true;
							}
							break;
						case MAP_CPPON_OBJ_TYPE:
							if( syncMap( jobj, l ) )
							{
								result = true;
							}
							break;
						case ARRAY_CPPON_OBJ_TYPE:
							if( syncArray( jobj, l ) )
							{
								result = true;
							}
							break;
						default:
							break;
					}
				}
			}
		}
	}
	return result;
}
bool SCppObj::syncArray( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;
	if( lst && SL_TYPE_ARRAY == lst->type &&  CppON::isArray( obj ) )
	{
		for( unsigned i = 0; (unsigned) ((COArray *) obj)->size() > i; i++ )
		{
			if( lst->nSubs > i )
			{
				CppON		*jobj = ((COArray *) obj)->at( i );
				STRUCT_LISTS *l = &((STRUCT_LISTS *) lst->subs)[ i ];
				switch( jobj->type() )
				{
					case INTEGER_CPPON_OBJ_TYPE:
						if( syncInt( jobj, l ) )
						{
							result = true;
						}
						break;
					case DOUBLE_CPPON_OBJ_TYPE:
						if( syncDouble( jobj, l ) )
						{
							result = true;
						}
						break;
					case STRING_CPPON_OBJ_TYPE:
						if( syncString( jobj, l ) )
						{
							result = true;
						}
						break;
					case BOOLEAN_CPPON_OBJ_TYPE:
						if( syncBoolean( jobj, l ) )
						{
							result = true;
						}
						break;
					case MAP_CPPON_OBJ_TYPE:
						if( syncMap( jobj, l ) )
						{
							result = true;
						}
						break;
					case ARRAY_CPPON_OBJ_TYPE:
						if( syncArray( jobj, l ) )
						{
							result = true;
						}
						break;
					default:
						break;
				}
			}
		}
	}
	return result;
}
bool SCppObj::sync( CppON *obj, STRUCT_LISTS *lst )
{
	bool result = false;
	if( CppON::isObj( obj ) && lst )
	{
		switch ( obj->type() )
		{
			case  INTEGER_CPPON_OBJ_TYPE:
				result = syncInt( obj, lst );
				break;
			case DOUBLE_CPPON_OBJ_TYPE:
				result = syncDouble( obj, lst );
				break;
			case STRING_CPPON_OBJ_TYPE:
				result = syncString( obj, lst );
				break;
			case BOOLEAN_CPPON_OBJ_TYPE:
				result = syncBoolean( obj, lst );
				break;
			case MAP_CPPON_OBJ_TYPE:
				result = syncMap( obj, lst );
				break;
			case ARRAY_CPPON_OBJ_TYPE:
				result = syncArray( obj, lst );
				break;
			default:
				break;

		}
	}
	return result;
}

bool SCppObj::waitSem( const char *path, STRUCT_LISTS *lst )
{
	STRUCT_LISTS 	*tst = getPointer( path, lst );
	if( tst )
	{
		return( waitSem( tst->sem ) );
	} else {
		fprintf( stderr, "%s[%.4u]: ERROR - Failed to get semaphore to wait on: '%s'\n",__FILE__ , __LINE__, path  );
	}
	return false;
}
bool SCppObj::postSem( const char *path, STRUCT_LISTS *lst )
{
	STRUCT_LISTS 	*tst = getPointer( path, lst );
	if( tst )
	{
		sem_post( tst->sem );
		return true;
	} else {
		fprintf( stderr, "%s[%.4u]: ERROR - Failed to get semaphore to Post too: '%s'\n",__FILE__ , __LINE__, path );
	}
	return false;
}

void SCppObj::setUpdateTime( const char *path, STRUCT_LISTS *lst, uint64_t t )
{
	STRUCT_LISTS 	*tst = getPointer( path, lst );
	if( tst )
	{
		setUpdateTime( tst, t );
	}
}

uint64_t SCppObj::getUpdateTime( const char *path, STRUCT_LISTS *lst )
{
	STRUCT_LISTS 	*tst = getPointer( path, lst );
	if( tst )
	{
		return getUpdateTime( tst );
	}
	return 0;
}

/*
 * We use timespec because our time is based on clock_gettime( CLOCK_MONOTONIC &tsp ) instead of gettimeofday( &tv, NULL );
 */
bool SCppObj::getUpdateTime( struct timespec &tsp, STRUCT_LISTS *lst )
{
	bool rtn = false;
	if( lst )
	{
		if( SL_TYPE_UNIT == lst->type )
		{
			for( unsigned i = 0; lst->nSubs > i; i++ )
			{
				if( getUpdateTime( tsp, &((STRUCT_LISTS *) lst->subs)[ i ] ) )
				{
					rtn = true;
				}
			}
		} else {
			uint64_t t = *((uint64_t *)((char*) basePtr + lst->time) );
			if( t )
			{
				uint32_t nsec = (uint32_t ) ( t % 1000 ) * 1000000;
				uint32_t sec = (uint32_t)( t / 1000 );
				if( (uint32_t) tsp.tv_sec < sec || ( (uint32_t)tsp.tv_sec == sec && (uint32_t)tsp.tv_nsec < nsec ) )
				{
					tsp.tv_sec = sec;
					tsp.tv_nsec = nsec;
				}
			} else {
				rtn = true;
			}
		}
	}
	return rtn;
}

STRUCT_LISTS *SCppObj::at( STRUCT_LISTS *lst, uint32_t idx )
{
	STRUCT_LISTS *rtn = NULL;
	if( lst && ( SL_TYPE_UNIT == lst->type || SL_TYPE_ARRAY == lst->type ) && idx < lst->nSubs )
	{
		rtn = &(lst->subs[ idx ] );
	}
	return rtn;
}

STRUCT_LISTS  *SCppObj::getElement( const char *path, STRUCT_LISTS *base )
{
	const char *e;
	const char *key;
	for( e = path; *e && '/' != *e && '.' != *e; e++ );
	unsigned l = e - path;
	if( path && base  && base->names )
	{
		std::string s( path, l );
		for( unsigned i = 0; ( key = base->names[ i ] ); i += 2 )
		{
			unsigned j;
			for( j = 0; j < l && key[ j ] && path[ j ] == key[ j ]; j++);
			if( ! key[ j ] )											// found
			{
				if( l != strlen( base->names[ i + 1 ] ) )												// Fail! Strings aren't same length
				{
				} else if( strncmp( base->names[ i + 1 ], path, l ) ) {									// Fail! Strings aren't equal
					break;
				} else if( *e ) {
					return getElement( ++e, &( ( (STRUCT_LISTS *) base->subs )[ i / 2 ] ) );			// Search for child
				} else {
					return &( ( (STRUCT_LISTS *) base->subs )[ i / 2 ] );								// Found IT!
				}
			} else if( key[ j ] > path[ j ] ) {															// Fail!  We passed it without finding it.
				break;
			}
		}
	}
	return NULL;
}

char *SCppObj::String( STRUCT_LISTS *val, char *var, int sz )
{
	char *rtn = NULL;
	if( val )
	{
		waitSem( val->sem );
		if( var && sz )
		{
			switch ( val->type )
			{
				case SL_TYPE_CHAR:
					strncpy( var, ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ), sz );
					break;
				case SL_TYPE_INT64:
#if SIXTY_FOUR_BIT
					snprintf( var, sz, "%lX", *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
#else
					snprintf( var, sz, "%llX", *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
#endif
					break;
				case SL_TYPE_INT32:
					snprintf( var, sz, "%X", *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					break;
				case SL_TYPE_INT16:
					snprintf( var, sz, "%X", (uint32_t) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					break;
				case SL_TYPE_INT8:
					snprintf( var, sz, "%X", (uint32_t) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					break;
				case SL_TYPE_DOUBLE:
					snprintf( var, sz, "%.lf", *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					break;
				case SL_TYPE_BOOL:
					snprintf( var, sz, "%s", ( 0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ) ? "True": "False" );
					break;
				case SL_TYPE_UNIT:
				case SL_TYPE_ARRAY:
				default:
					break;
			}
			rtn = var;
		} else {
			char buffer[ 64 ];
			switch ( val->type )
			{
				case SL_TYPE_CHAR:
						rtn = strdup( ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					break;
				case SL_TYPE_INT64:
#if SIXTY_FOUR_BIT
					snprintf( buffer, 63, "%lX", *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
#else
					snprintf( buffer, 63, "%llX", *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
#endif
					rtn = strdup( buffer );
					break;
				case SL_TYPE_INT32:
					snprintf( buffer, 63, "%X", *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					rtn = strdup( buffer );
					break;
				case SL_TYPE_INT16:
					snprintf( buffer, 63, "%X", (uint32_t) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					rtn = strdup( buffer );
					break;
				case SL_TYPE_INT8:
					snprintf( buffer, 63, "%X", (uint32_t) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					rtn = strdup( buffer );
					break;
				case SL_TYPE_DOUBLE:
					snprintf( buffer, 63, "%lf", *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
					rtn = strdup( buffer );
					break;
				case SL_TYPE_BOOL:
					rtn = strdup( ( 0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ) ? "True": "False" );
					break;
				case SL_TYPE_UNIT:
				case SL_TYPE_ARRAY:
				default:
					break;
			}
		}
		sem_post( val->sem );
	}
	return rtn;
}

uint64_t SCppObj::toLong( STRUCT_LISTS *val )
{
	uint64_t rtn = 0;
	if( val )
	{
		waitSem( val->sem );

		switch ( val->type )
		{
			case SL_TYPE_INT64:
				rtn = *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT32:
				rtn = (uint64_t) *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT16:
				rtn = (uint64_t) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT8:
				rtn = (uint64_t) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_DOUBLE:
				rtn = (uint64_t) *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_CHAR:
				rtn = strtoll( ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ), NULL, 0 );
				break;
			case SL_TYPE_BOOL:
				rtn = (0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ) ? 1 : 0;
				break;
			default:
				break;
		}
		sem_post( val->sem );
	}
	return rtn;
}
uint32_t SCppObj::Int( STRUCT_LISTS *val )
{
	uint32_t rtn = 0;
	if( val )
	{
		waitSem( val->sem );

		switch ( val->type )
		{
			case SL_TYPE_INT32:
				rtn = (uint32_t) *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT16:
				rtn = (uint32_t) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT8:
				rtn = (uint32_t) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_DOUBLE:
				rtn = (uint32_t) *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT64:
				rtn = *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_CHAR:
				rtn = (uint32_t) strtol( ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ), NULL, 0 );
				break;
			case SL_TYPE_BOOL:
				rtn = (0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ) ? 1 : 0;
				break;
			default:
				break;
		}
		sem_post( val->sem );
	}
	return rtn;
}
double SCppObj::toDouble( STRUCT_LISTS *val )
{
	double rtn = 0.0;
	if( val )
	{
		waitSem( val->sem );

		switch ( val->type )
		{
			case SL_TYPE_DOUBLE:
				rtn =  *( ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT64:
				rtn = (double) *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT32:
				rtn = (double) *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT16:
				rtn = (double) *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_INT8:
				rtn = (double) *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) );
				break;
			case SL_TYPE_CHAR:
				rtn =  strtod( ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ), NULL );
				break;
			case SL_TYPE_BOOL:
				rtn = (0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) ) ? 1.0 : 0.0;
				break;
			default:
				break;
		}
		sem_post( val->sem );
	}
	return rtn;
}

bool SCppObj::toBoolean( STRUCT_LISTS *val )
{
	bool rtn = false;
	if( val )
	{
		waitSem( val->sem );

		switch ( val->type )
		{
			case SL_TYPE_BOOL:
				rtn = (0 != *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
				break;
			case SL_TYPE_INT64:
				rtn =   (  0 != *(  ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
				break;
			case SL_TYPE_INT32:
				rtn =   (  0 != *(  ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
				break;
			case SL_TYPE_INT16:
				rtn =   (  0 != *(  ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
				break;
			case SL_TYPE_INT8:
				rtn =   (  0 != *(  ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
				break;
			case SL_TYPE_DOUBLE:
				rtn =   (  0.0 != *(  ( double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ) ) );
				break;
			case SL_TYPE_CHAR:
				rtn = ( 0 == strcasecmp( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) val->offset ), "True" ) );
				break;
			default:
				break;
		}
		sem_post( val->sem );
	}
	return rtn;
}

double SCppObj::doubleValue( STRUCT_LISTS *tst, bool protect, bool *valid )
{
	double result = 0.0;
	if( tst )
	{
		if( protect )
		{
			waitSem( tst->sem );
		}
		if( valid )
		{
			*valid = true;
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				result = strtod( ( ( char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ), NULL );
				break;
			case SL_TYPE_DOUBLE:
				result = *((double *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT64:
				result = (double) *( ( int64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT32:
				result = (double) *( ( int32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT8:
				result = (double) *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_BOOL:
				result = ( 0 != *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? 1.0 : 0.0;
				break;
			default:
				if( valid )
				{
					*valid = false;
				}
				break;
		}
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}
	} else {
		if( valid )
		{
			*valid = false;
		}
	}
	return result;
}
double *SCppObj::readDouble( const char *path, double *result, bool protect, STRUCT_LISTS *lst )
{
	bool 		valid = false;
	double 	r = doubleValue( path,  protect, lst, &valid );
	if( valid )
	{
		if( result )
		{
			*result = r;
		} else {
			result = new double( r );
		}
	} else {
		result = NULL;
	}
	return result;
}

uint64_t SCppObj::longValue( STRUCT_LISTS *tst, bool protect, bool *valid )
{
	uint64_t result = 0;
	if( tst )
	{
		if( protect )
		{
			waitSem( tst->sem );
		}
		if( valid )
		{
			*valid = true;
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				result = strtoll( ( ( char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ), NULL, 0 );
				break;
			case SL_TYPE_DOUBLE:
				result =  (int64_t) *((double *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT64:
				result = (uint64_t) *( ( int64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT32:
				result = (uint64_t) *( ( int32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT8:
				result = (uint64_t) *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_BOOL:
				result = ( 0 != *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? 1LL : 0LL;
				break;
			default:
				if( valid )
				{
					*valid = false;
				}
				break;
		}

		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}
	} else {
		if( valid )
		{
			*valid = false;
		}
	}
	return result;
}

uint64_t *SCppObj::readLong( const char *path, uint64_t *result, bool protect, STRUCT_LISTS *lst )
{
	bool 		valid = false;
	uint64_t 	r = longValue( path,  protect, lst, &valid );
	if( valid )
	{
		if( result )
		{
			*result = r;
		} else {
			result = new uint64_t( r );
		}
	} else {
		result = NULL;
	}
	return result;
}

uint32_t SCppObj::intValue( STRUCT_LISTS *tst, bool protect, bool *valid )
{
	uint32_t result = 0;
	if( tst )
	{
		if( protect )
		{
			waitSem( tst->sem );
		}
		if( valid )
		{
			*valid = true;
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				result = strtol( ( ( char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ), NULL, 0 );
				break;
			case SL_TYPE_DOUBLE:
				result =  (int32_t) *((double *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT64:
				result = (uint32_t) *( ( int64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT32:
				result = (uint32_t) *( ( int32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT16:
				result = (uint32_t) *( ( int16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_INT8:
				result = (uint32_t) *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_BOOL:
				result = ( 0 != *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? 1 : 0;
				break;
			default:
				if( valid )
				{
					*valid = false;
				}
				break;
		}
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}
	} else {
		if( valid )
		{
			*valid = false;
		}
	}
	return result;
}
uint32_t *SCppObj::readInt( const char *path, uint32_t *result, bool protect, STRUCT_LISTS *lst )
{
	bool 		valid = false;
	uint32_t 	r = intValue( path,  protect, lst, &valid );
	if( valid )
	{
		if( result )
		{
			*result = r;
		} else {
			result = new uint32_t( r );
		}
	} else {
		result = NULL;
	}
	return result;
}

bool SCppObj::boolValue( STRUCT_LISTS *tst, bool protect, bool *valid )
{
	bool result = false;
	if( tst )
	{
		if( protect )
		{
			waitSem( tst->sem );
		}
		if( valid )
		{
			*valid = true;
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				result = ( 0 == strcasecmp( ( ( char *)( (uint64_t ) basePtr ) + (uint64_t ) tst->offset ),"true" ) );
				break;
			case SL_TYPE_DOUBLE:
				result = ( 0.0 != *((double *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				break;
			case SL_TYPE_INT64:
				result = ( 0 != *( ( int64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				break;
			case SL_TYPE_INT32:
				result = ( 0 != *( ( int32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				break;
			case SL_TYPE_INT8:
				result = ( 0 != *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				break;
			case SL_TYPE_BOOL:
				result = ( 0 != *( ( int8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				break;
			default:
				if( valid )
				{
					*valid = false;
				}
				break;
		}
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}
	} else {
		if( valid )
		{
			*valid = false;
		}
	}
	return result;
}
bool *SCppObj::readBool( const char *path, bool *result, bool protect, STRUCT_LISTS *lst )
{
	bool 		valid = false;
	bool 		r = intValue( path,  protect, lst, &valid );
	if( valid )
	{
		if( result )
		{
			*result = r;
		} else {
			result = new bool( r );
		}
	} else {
		result = NULL;
	}
	return result;
}

const char *SCppObj::readBase64String( STRUCT_LISTS *tst, std::string *result, bool protect )
{
	if( tst && result)
	{
		char 		buffer[ 64 ];
		buffer[ 63 ] = '\0';

		result->clear();
		if( protect )
		{
			waitSem( tst->sem );
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				{
					std::string *val = COString::toBase64JsonString( (const char *) ( (char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ), (unsigned int) tst->size );
					if( val )
					{
						fprintf( stderr, "%s\n", val->c_str() );
						*result = *val;
						delete val;
					}
				}
				break;
			case SL_TYPE_DOUBLE:
				{
					CppON 	*jObj;
					char 		fmt[ 8 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 7 ] = '\0';
						snprintf( fmt, 7, "%%.%dlf", jObj->toInt() );
					} else {
						strcpy( fmt, "%lf" );
					}
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, fmt,*((double*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					*result = buffer;
				}
				break;
			case SL_TYPE_INT64:
				{
					CppON 	*jObj;
					char 		fmt[ 12 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 11 ] = '\0';
#if SIXTY_FOUR_BIT
						snprintf( fmt, 11, "%%.%dlX", jObj->toInt() );
#else
						snprintf( fmt, 11, "%%.%dllX", jObj->toInt() );
#endif
					} else {
						strcpy( fmt, "0x%.12llX" );
					}
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, fmt,*((uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					*result = buffer;
				}
				break;
			case SL_TYPE_INT32:
				{
					CppON 	*jObj;
					char 		fmt[ 12 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 11 ] = '\0';
						snprintf( fmt, 11, "%%.%dX", jObj->toInt() );
					} else {
						strcpy( fmt, "0x%.8X" );
					}
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, fmt,*((uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					*result = buffer;
				}
				break;
			case SL_TYPE_INT16:
				snprintf( buffer, 63, "0x%.4X",(uint16_t) *((uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				*result = buffer;
				break;
			case SL_TYPE_INT8:
				snprintf( buffer, 63, "0x%.2X",(uint8_t) *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				*result = buffer;
				break;
			case SL_TYPE_BOOL:
				sprintf( buffer,"%s", ( 0  != *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? "True" : "False" );
				*result = buffer;
				break;
			default:
				break;
		}
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}
	}
    return result->c_str();
}

const char *SCppObj::readString( STRUCT_LISTS *tst, std::string *result, bool protect )
{
	if( tst && result)
	{
		char 		buffer[ 64 ];
		buffer[ 63 ] = '\0';

		result->clear();
		if( protect )
		{
			waitSem( tst->sem );
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				*result = ( (char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				break;
			case SL_TYPE_DOUBLE:
				{
					CppON 	*jObj;
					char 		fmt[ 8 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 7 ] = '\0';
						snprintf( fmt, 7, "%%.%dlf", jObj->toInt() );
					} else {
						strcpy( fmt, "%lf" );
					}
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, fmt,*((double*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					*result = buffer;
				}
				break;
			case SL_TYPE_INT64:
				{
					CppON 	*jObj;
					char 		fmt[ 12 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 11 ] = '\0';
#if SIXTY_FOUR_BIT
						snprintf( fmt, 11, "%%.%dllX", jObj->toInt() );
#else
						snprintf( fmt, 11, "%%.%dllX", jObj->toInt() );
#endif
					} else {
#if SIXTY_FOUR_BIT
						strcpy( fmt, "0x%.12lX" );
#else
						strcpy( fmt, "0x%.12llX" );
#endif
					}
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, fmt,*((uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					*result = buffer;
				}
				break;
			case SL_TYPE_INT32:
				{
					CppON 	*jObj;
					char 		fmt[ 12 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 11 ] = '\0';
						snprintf( fmt, 11, "%%.%dX", jObj->toInt() );
					} else {
						strcpy( fmt, "0x%.8X" );
					}
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, fmt,*((uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					*result = buffer;
				}
				break;
			case SL_TYPE_INT16:
				snprintf( buffer, 63, "0x%.4X",(uint16_t) *((uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				*result = buffer;
				break;
			case SL_TYPE_INT8:
				snprintf( buffer, 63, "0x%.2X",(uint8_t) *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				*result = buffer;
				break;
			case SL_TYPE_BOOL:
				sprintf( buffer,"%s", ( 0  != *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? "True" : "False" );
				*result = buffer;
				break;
			default:
				break;
		}
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}
	}
    return result->c_str();
}


/*
 * Size is number of bytes in storage space
 */


char *SCppObj::readString( const char *path, char *result, size_t sz, bool protect, STRUCT_LISTS *lst )
{
	STRUCT_LISTS 	*tst = getPointer( path, lst );
	if( tst )
	{

		if( protect )
		{
			waitSem( tst->sem );
		}
		switch ( tst->type )
		{
			case SL_TYPE_CHAR:
				if( !sz || ! result )
				{
					result = strdup( (char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) );
				} else {
					sz--;
					result[ sz ] = '\0';
					strncpy( result, (char *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ), sz );
				}
				break;
			case SL_TYPE_DOUBLE:
				{
					CppON 	*jObj;
					char 		fmt[ 8 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 7 ] = '\0';
						snprintf( fmt, 7, "%%.%dlf", jObj->toInt() );
					} else {
						strcpy( fmt, "%lf" );
					}
					if( !sz || ! result )
					{
						char buffer[ 64 ];
						buffer[ 63 ] = '\0';
						snprintf( buffer, 63, fmt,*((double*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
						result = strdup( buffer );
					} else {
						sz--;
						result[ sz ] = '\0';
						snprintf( result, sz, fmt,*((double*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					}
				}
				break;
			case SL_TYPE_INT64:
				{
					CppON 	*jObj;
					char 		fmt[ 12 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 11 ] = '\0';
#if SIXTY_FOUR_BIT
						snprintf( fmt, 11, "%%.%dlX", jObj->toInt() );
#else
						snprintf( fmt, 11, "%%.%dllX", jObj->toInt() );
#endif
					} else {
#if SIXTY_FOUR_BIT
						strcpy( fmt, "0x%.12lX" );
#else
						strcpy( fmt, "0x%.12llX" );
#endif
					}
					if( !sz || ! result )
					{
						char buffer[ 64 ];
						buffer[ 63 ] = '\0';
						snprintf( buffer, 63, fmt,*((uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
						result = strdup( buffer );
					} else {
						sz--;
						result[ sz ] = '\0';
						snprintf( result, sz, fmt,*((uint64_t*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					}
				}
				break;
			case SL_TYPE_INT32:
				{
					CppON 	*jObj;
					char 		fmt[ 12 ];
					if( CppON::isMap( tst->def ) && (jObj = tst->def->findElement( "precision" ) ) )
					{
						fmt[ 11 ] = '\0';
						snprintf( fmt, 11, "%%.%dX", jObj->toInt() );
					} else {
						strcpy( fmt, "0x%.8X" );
					}
					if( !sz || ! result )
					{
						char buffer[ 64 ];
						buffer[ 63 ] = '\0';
						snprintf( buffer, 63, fmt,*((uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
						result = strdup( buffer );
					} else {
						sz--;
						result[ sz ] = '\0';
						snprintf( result, sz, fmt,*((uint32_t*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					}
				}
				break;
			case SL_TYPE_INT16:
				if( !sz || ! result )
				{
					char buffer[ 64 ];
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, "0x%.4X",(uint16_t) *((uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					result = strdup( buffer );
				} else {
					sz--;
					result[ sz ] = '\0';
					snprintf( result, sz, "0x%.4X",(uint16_t) *((uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				}
				break;
			case SL_TYPE_INT8:
				if( !sz || ! result )
				{
					char buffer[ 64 ];
					buffer[ 63 ] = '\0';
					snprintf( buffer, 63, "0x%.2X",(uint8_t) *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
					result = strdup( buffer );
				} else {
					sz--;
					result[ sz ] = '\0';
					snprintf( result, sz, "0x%.2X",(uint8_t)*((uint8_t*)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) );
				}
				break;
			case SL_TYPE_BOOL:
				if( !sz || ! result )
				{
					char buffer[ 8 ];
					buffer[ 7 ] = '\0';
					sprintf( buffer,"%s", ( 0  != *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? "True" : "False" );
					result = strdup( buffer );
				} else {
					sz--;
					result[ sz ] = '\0';
					snprintf( result, sz, "%s", ( 0  != *((uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) tst->offset ) ) ) ? "True" : "False" );
				}
				break;
			default:
				result = NULL;
				break;
		}
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( tst->sem );
		}

	} else {
		if( result )
		{
			result[ 0 ] = '\0';
		}
		fprintf( stderr, "%s[%.4d]: Could not find '%s' to read as string\n", __FILE__, __LINE__, path  );
		if( lst )
		{
			fprintf( stderr, "%s[%.4d]: Looking in %s\n", __FILE__, __LINE__, lst->name.c_str()  );
		}
		result = NULL;
	}
    return result;
}


/*
 * update a string in memory with a new value:
 * 		UpdateString( path, str, protect lst );
 * 			path 		- Required. Path to variable
 * 			str 		- Required. The falue you are setting it to.
 * 			protect 	- Optional. Default = true.  True if mulit-thread protection is wanted. Warning: DO NOT SET if you already have semaphore
 * 			lst			- Optional. Default = base.  This allows the search to start at a lower level in the class structure
 */

bool SCppObj::updateString( STRUCT_LISTS *lst, const char *str, bool protect )
{
	bool 			rtn = true;
	if( lst )
	{
		if( protect )
		{
			waitSem( lst->sem );
		}
		switch( lst->type )
		{
			case SL_TYPE_CHAR:
				((char *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ))[ lst->size - 1 ] = '\0';
				strncpy( (char *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ), str, lst->size - 1 );
				break;
			case SL_TYPE_DOUBLE:
				*( (double *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = strtod( str, NULL );
				break;
			case SL_TYPE_INT64:
				*( (uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint64_t) strtoll( str, NULL, 0 );
				break;
			case SL_TYPE_INT32:
				*( (uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint32_t) strtol( str, NULL, 0 );
				break;
			case SL_TYPE_INT16:
				*( (uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint16_t ) strtol( str, NULL, 0 );
				break;
			case SL_TYPE_INT8:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint8_t) strtol( str, NULL, 0 );
				break;
			case SL_TYPE_BOOL:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( ! strcasecmp( str, "True") ) ? 0xFF: 0x00;
				break;
			default:
				rtn = false;
				break;
		}
		setUpdateTime( lst );
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( lst->sem );
		}
	} else {
		rtn = false;
	}
	return rtn;

}

bool SCppObj::update( STRUCT_LISTS *lst, void *obj, bool protect  )
{
	bool			rtn = false;

	if( lst )
	{
		switch( lst->type )
		{
			case SL_TYPE_CHAR:
				return updateString( lst, (const char *)obj, protect );
				break;
			case SL_TYPE_DOUBLE:
				return updateDouble( lst, *( (double *)obj ), protect );
				break;
			case SL_TYPE_INT64:
				return updateLong( lst, *( (uint64_t *)obj ), protect );
				break;
			case SL_TYPE_INT32:
				if( updateInt( lst, *( (uint32_t *)obj ), protect ) )
				{
					return true;
				} else {
					fprintf(stderr, "%d: Failed to update %s\n", __LINE__, lst->name.c_str() );
					return false;
				}
				break;
			case SL_TYPE_INT16:
				if( updateInt( lst, (uint32_t) *( (uint16_t *)obj ), protect ) )
				{
					return true;
				} else {
					fprintf(stderr, "%d: Failed to update %s\n", __LINE__, lst->name.c_str() );
					return false;
				}
				break;
			case SL_TYPE_INT8:
				if(updateInt( lst, (uint32_t) *( (uint8_t *)obj ), protect ) )
				{
					return true;
				} else {
					fprintf(stderr, "%d: Failed to update %s\n", __LINE__, lst->name.c_str() );
					return false;
				}
				break;
			case SL_TYPE_BOOL:
				return updateBoolean( lst, (uint32_t) *( (bool *)obj ), protect );
				break;
			case SL_TYPE_UNIT:
				return updateObject( lst, (COMap *) obj, protect );
				break;
			case SL_TYPE_ARRAY:
				return updateArray( lst, (COArray *) obj, protect );
				break;
			default:
				break;
		}
	}
	return rtn;
}
bool SCppObj::updateDouble( STRUCT_LISTS *lst, double val, bool protect )
{
	bool 			rtn = true;
	if(  lst )
	{
		if( protect )
		{
			waitSem( lst->sem );
		}
		switch( lst->type )
		{
			case SL_TYPE_CHAR:
				{
					CppON *oPtr = lst->def->findCaseElement( "size" );
					char 	*cPtr = (char *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset );
					int sz = 8;
					if( CppON::isNumber( oPtr ) )
					{
						sz = oPtr->toInt();
					}
					sz--;
					cPtr[ sz ] = '\0';
					snprintf( cPtr, sz, "%lf", val );
				}
				break;
			case SL_TYPE_DOUBLE:
				*( (double *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (double) val;
				break;
			case SL_TYPE_INT64:
				*( (uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (int64_t) round( val );
				break;
			case SL_TYPE_INT32:
				*( (uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (int32_t) round( val );
				break;
			case SL_TYPE_INT16:
				*( (uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (int16_t ) round( val );
				break;
			case SL_TYPE_INT8:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint8_t) round( val );
				break;
			case SL_TYPE_BOOL:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( 0.0 != val ) ? 0xFF: 0x00;
				break;
			default:
				rtn = false;
				break;

		}
		setUpdateTime( lst );
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( lst->sem );
		}
	} else {
		rtn = false;
	}
	return rtn;
}
bool SCppObj::updateLong( STRUCT_LISTS *lst, uint64_t val, bool protect )
{
	bool 			rtn = true;

	if(  lst )
	{
		if( protect )
		{
			waitSem( lst->sem );
		}
		switch( lst->type )
		{
			case SL_TYPE_CHAR:
				{
					CppON *oPtr = lst->def->findCaseElement( "size" );
					char 	*cPtr = (char *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset );
					int sz = 8;
					if( CppON::isNumber( oPtr ) )
					{
						sz = oPtr->toInt();
					}
					sz--;
					cPtr[ sz ] = '\0';
#if SIXTY_FOUR_BIT
					snprintf( cPtr, sz, "0x%lX", val );
#else
					snprintf( cPtr, sz, "0x%llX", val );
#endif
				}
				break;
			case SL_TYPE_DOUBLE:
				*( (double *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (double) val;
				break;
			case SL_TYPE_INT64:
				*( (uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint64_t) val;
				break;
			case SL_TYPE_INT32:
				*( (uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint32_t) val;
				break;
			case SL_TYPE_INT16:
				*( (uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint16_t ) val;
				break;
			case SL_TYPE_INT8:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint8_t) val;
				break;
			case SL_TYPE_BOOL:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( 0 != val ) ? 0xFF: 0x00;
				break;
			default:
				rtn = false;
				break;

		}
		setUpdateTime( lst );
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( lst->sem );
		}
	} else {
		rtn = false;
	}
	return rtn;
}
bool SCppObj::updateInt( STRUCT_LISTS *lst, uint32_t val, bool protect )
{
	bool 			rtn = true;

	if(  lst )
	{
		if( protect )
		{
			waitSem( lst->sem );
		}
		switch( lst->type )
		{
			case SL_TYPE_CHAR:
				{
					CppON *oPtr = lst->def->findCaseElement( "size" );
					char 	*cPtr = (char *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset );
					int sz = 8;
					if( CppON::isNumber( oPtr ) )
					{
						sz = oPtr->toInt();
					}
					sz--;
					cPtr[ sz ] = '\0';
					snprintf( cPtr, sz, "0x%X", val );
				}
				break;
			case SL_TYPE_DOUBLE:
				*( (double *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (double) val;
				break;
			case SL_TYPE_INT64:
				*( (uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint64_t) val;
				break;
			case SL_TYPE_INT32:
				*( (uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint32_t ) val;
				break;
			case SL_TYPE_INT16:
				*( (uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint16_t ) val;
				break;
			case SL_TYPE_INT8:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint8_t) val;
				break;
			case SL_TYPE_BOOL:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( 0 != val ) ? 0xFF: 0x00;
				break;
			default:
				rtn = false;
				break;

		}
		setUpdateTime( lst );
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( lst->sem );
		}
	} else {
		fprintf( stderr, "%s[%d]: Failed to update integer because it wasn't found\n",__FILE__, __LINE__ );
		rtn = false;
	}
	return rtn;
}
bool SCppObj::updateBoolean( STRUCT_LISTS *lst, bool val, bool protect )
{
	bool 			rtn = true;

	if(  lst )
	{
		if( protect )
		{
			waitSem( lst->sem );
		}
		switch( lst->type )
		{
			case SL_TYPE_CHAR:
				{
					CppON *oPtr = lst->def->findCaseElement( "size" );
					char 	*cPtr = (char *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset );
					int sz = 8;
					if( CppON::isNumber( oPtr ) )
					{
						sz = oPtr->toInt();
					}
					sz--;
					cPtr[ sz ] = '\0';
					snprintf( cPtr, sz, "%s", ( val ) ? "True":"False" );
				}
				break;
			case SL_TYPE_DOUBLE:
				*( (double *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( val ) ? 1.0 : 0.0;
				break;
			case SL_TYPE_INT64:
				*( (uint64_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( val ) ? 1LL : 0LL;
				break;
			case SL_TYPE_INT32:
				*( (uint32_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( val ) ? 1 : 0;
				break;
			case SL_TYPE_INT16:
				*( (uint16_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = ( val ) ? 1:0;
				break;
			case SL_TYPE_INT8:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = (uint8_t) val;
				break;
			case SL_TYPE_BOOL:
				*( (uint8_t *)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) = val;
				break;
			default:
				rtn = false;
				break;

		}
		setUpdateTime( lst );
		if( protect )																											// If protected post even if we didn't get the semaphore assuming a stuck one.
		{
			sem_post( lst->sem );
		}
	} else {
		rtn = false;
	}
	return rtn;
}
bool SCppObj::updateArray( STRUCT_LISTS *lst, COArray *arr, bool protect )
{
	if(  lst && CppON::isArray( arr ) )
	{
		unsigned max = lst->nSubs;
		for( unsigned i = 0; max > i && arr->size(); i++ )
		{
			STRUCT_LISTS	*val = &lst->subs[ i ];
			CppON			*obj = arr->at( i );
			int typ = val->type;
			switch ( obj->type() )
			{
				case INTEGER_CPPON_OBJ_TYPE:
					switch( typ )
					{
						case SL_TYPE_INT64:
							updateLong( val, obj->toLongInt(), protect );
							break;
						case SL_TYPE_INT32:
							updateInt( val, obj->toInt(), protect );
							break;
						case SL_TYPE_INT16:
							updateInt( val, (int32_t) obj->toInt(), protect );
							break;
						case SL_TYPE_INT8:
							updateInt( val, (int32_t) obj->toInt(), protect );
							break;
						default:
							break;
					}
					break;
				case DOUBLE_CPPON_OBJ_TYPE:
					if( SL_TYPE_DOUBLE == typ )
					{
						updateDouble( val, obj->toDouble(), protect  );
					}
					break;
				case STRING_CPPON_OBJ_TYPE:
					if( SL_TYPE_CHAR == typ )
					{
						updateString( val, obj->c_str(), protect  );
					}
					break;
				case BOOLEAN_CPPON_OBJ_TYPE:
					if( SL_TYPE_BOOL == typ )
					{
						updateBoolean( val,((COBoolean *)obj)->value(), protect );
					}
					break;
				case MAP_CPPON_OBJ_TYPE:
					if( SL_TYPE_UNIT == typ )
					{
						updateObject( val, (COMap *)obj, protect );
					}
					break;
				case ARRAY_CPPON_OBJ_TYPE:
					if( SL_TYPE_ARRAY == typ )
					{
						updateArray( val, (COArray *)obj, protect );
					}
					break;
				default:
					break;
			}

		}
	}
	return false;
}
bool SCppObj::updateObject( STRUCT_LISTS *lst, COMap *obj, bool protect  )
{
	if(  lst && CppON::isMap( obj ) )
	{
		for( std::map<std::string, CppON *>::iterator it = obj->begin(); obj->end() != it; it++ )
		{
			CppON		&ob = *(it->second);

			unsigned 	i;
			for( i = 0;  lst->nSubs > i; i++ )
			{
				STRUCT_LISTS *obj = &( (STRUCT_LISTS *) lst->subs)[ i ];
				if( ! obj->name.compare( it->first.c_str() ) )
				{
					switch ( ob.type() )
					{
						case INTEGER_CPPON_OBJ_TYPE:
							if( sizeof( uint64_t ) == lst->size )
							{
								updateLong( obj, ob.toLongInt(), protect );
							} else {
								if( ! updateInt( obj, ob.toInt(), protect ) )
								{
									fprintf(stderr, "%d: Failed to update %s\n",__LINE__, obj->name.c_str() );
								}
							}
							break;
						case DOUBLE_CPPON_OBJ_TYPE:
							updateDouble( obj, ob.toDouble(), protect  );
							break;
						case STRING_CPPON_OBJ_TYPE:
							updateString( obj, ob.c_str(), protect  );
							break;
						case BOOLEAN_CPPON_OBJ_TYPE:
							updateBoolean( obj, ((COBoolean*) (it->second))->value(), protect  );
							break;
						case MAP_CPPON_OBJ_TYPE:
							updateObject( obj, (COMap *)&ob, protect );
							break;
						case ARRAY_CPPON_OBJ_TYPE:
							updateArray( obj, (COArray *)&ob, protect );
							break;
						default:
							break;
					}
					break;
				}
			}
		}
		return true;
	}
	return false;
}

bool SCppObj::update( CppON *obj, STRUCT_LISTS *lst )
{
	if( CppON::isObj( obj ) )
	{
		if( ! lst )
		{
			lst = list;
		}
		uint8_t typ = lst->type;

		switch( obj->type() )
		{
			case INTEGER_CPPON_OBJ_TYPE:
				switch( typ )
				{
					case SL_TYPE_INT64:
						return updateLong( lst,((COInteger *)obj)->toLongInt() );
						break;
					case SL_TYPE_INT32:
					case SL_TYPE_INT16:
					case SL_TYPE_INT8:
						if( ! updateInt( lst,((COInteger *)obj)->toInt() ) )
						{
							fprintf(stderr, "%d: Failed to update %s\n", __LINE__, lst->name.c_str() );
							return false;
						} else {
							return true;
						}
						break;
					default:
						break;
				}
				break;
			case DOUBLE_CPPON_OBJ_TYPE:
				if( SL_TYPE_DOUBLE == typ )
				{
					return updateDouble( lst,((CODouble *)obj)->toDouble() );
				}
				break;
			case STRING_CPPON_OBJ_TYPE:
				if( SL_TYPE_CHAR == typ )
				{
					return updateString( lst, ((COString *)obj)->c_str() );
				}
				break;
			case NULL_CPPON_OBJ_TYPE:
				break;
			case BOOLEAN_CPPON_OBJ_TYPE:
				if( SL_TYPE_BOOL == typ )
				{
					return updateBoolean( lst,((COBoolean *)obj)->value() );
				}
				break;
			case MAP_CPPON_OBJ_TYPE:
				if( SL_TYPE_UNIT == typ )
				{
					return updateObject( lst, (COMap *) obj );
				}
				break;
			case ARRAY_CPPON_OBJ_TYPE:
				if( SL_TYPE_ARRAY == typ )
				{
					return updateArray( lst, (COArray *) obj );
				}
				break;
			default:
				return false;
		}
	}
	return false;
}

bool SCppObj::waitForUpdate( STRUCT_LISTS *lst,  uint64_t start, uint64_t to )
{
	struct timespec		tsp;
	uint64_t			now;
	bool 				rtn = false;

	clock_gettime( CLOCK_MONOTONIC, &tsp);
	now = ((uint64_t) tsp.tv_sec ) * 1000LL + (uint64_t)( (( 500000 + tsp.tv_nsec ) / 1000000) );
	if( ! start )
	{
		start = now;
	}
	to += now;

	if( lst )
	{
		do
		{
			if( *((uint64_t *)((char*) basePtr + lst->time ) ) > start )
			{
				rtn = true;
				break;
			}
            usleep( 50 );
			clock_gettime( CLOCK_MONOTONIC, &tsp);
			now = ((uint64_t) tsp.tv_sec ) * 1000LL + (uint64_t)( (( 500000 + tsp.tv_nsec ) / 1000000) );
		} while( ! rtn && to > now );
	}
	return rtn;
}

bool SCppObj::equals( CppON &obj, STRUCT_LISTS *lst )
{
    bool rtn = false;

    if( lst )
    {
        CppONType typ = jsonType( lst );
        if( typ == obj.type() )
        {
            switch( typ )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                	switch( lst->type )
                	{
                		case SL_TYPE_INT64:
                            rtn = ( *( ( uint64_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) == (uint64_t) ((COInteger *) &obj )->longValue() );
                			break;
                		case SL_TYPE_INT32:
                            rtn = ( *( ( uint32_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) == (uint32_t) ((COInteger *) &obj )->intValue() );
                			break;
                		case SL_TYPE_INT16:
                            rtn = ( *( ( uint16_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) == (uint16_t) ((COInteger *) &obj )->intValue() );
                			break;
                		case SL_TYPE_INT8:
                            rtn = ( *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) == (uint8_t) ((COInteger *) &obj )->intValue() );
                			break;
                		default:
                			rtn = false;
                			break;
                	}
                    break;

                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    rtn = ( *( (double *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) == ((CODouble *) &obj)->doubleValue() );
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                	rtn = ( 0 == strcmp( ( ( char *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ), ((COString *) &obj )->c_str( ) ) );
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    rtn = ( ( *( ( uint8_t *) ( ( (uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) != 0 ) == ((COBoolean *) &obj)->value() );
                    break;
                case MAP_CPPON_OBJ_TYPE:
                	{
                    	unsigned i;
                    	CppON *el;
                    	if( lst->nSubs != ((COMap *) &obj )->size() )
                    	{
                    		break;
                    	}
                    	for( i = 0; lst->nSubs > i; i++ )
                    	{
                    		STRUCT_LISTS *s = &((STRUCT_LISTS *) lst->subs )[ i ];

                    		if( !( el = ( ( (COMap *) &obj )->findElement( s->name.c_str() ) ) ) || ! equals( *el , s ) )
                    		{
                    			break;
                    		}
                    	}
                    	rtn = ( i == lst->nSubs );
                        break;
                	}
                case ARRAY_CPPON_OBJ_TYPE:
                	{
                    	unsigned i;
                    	CppON *el;
                    	if( lst->nSubs != ((COMap *) &obj )->size() )
                    	{
                    		break;
                    	}
                    	for( i = 0; lst->nSubs > i; i++ )
                    	{
                    		if( !( el = ( (COArray *) &obj )->at( i ) ) || ! equals( *el,  &( ( STRUCT_LISTS *) lst->subs)[ i ] ) )
                    		{
                    			break;
                    		}
                    	}
                    	rtn = ( i == lst->nSubs );
                	}
                    break;
                default:
                    break;
            }
    	} else {
    		fprintf( stderr, "%s[%.4d]: %s !=  because type %d != %d\n", __FILE__, __LINE__, lst->name.c_str(), typ, obj.type() );
    	}
    }
    return rtn;
}

void SCppObj::deleteStructList( STRUCT_LISTS *lst, std::string indent )
{
	indent += '\t';
	if( SL_TYPE_UNIT == lst->type || SL_TYPE_ARRAY == lst->type )
	{
		unsigned i;
		for( unsigned units = 0; lst->nSubs > units; units++ )
		{
			STRUCT_LISTS *l = &( (STRUCT_LISTS *) lst->subs )[ units ];
			if( l )
			{
				deleteStructList( l, indent );
			}
		}
		for( i = 0; lst->names[ i ]; i++ )
		{
			delete[] (lst->names[ i ]);
		}
		delete[] lst->names;
		delete[] ((STRUCT_LISTS *) lst->subs);
	}
}

void SCppObj::printStructList( STRUCT_LISTS *lst, std::string indent )
{
	indent += '\t';
	fprintf( stderr, "%s[%.4d]: %s%s, type: %d => ", __FILE__, __LINE__, indent.c_str(), lst->name.c_str(), (unsigned) lst->type );
	switch( lst->type )
	{
		case SL_TYPE_DOUBLE:
			fprintf( stderr, "%lf\n",*((double*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
			break;
		case SL_TYPE_INT64:
#if SIXTY_FOUR_BIT
			fprintf( stderr, "%.8llX\n",*((uint64_t*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
#else
			fprintf( stderr, "%.8llX\n",*((uint64_t*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
#endif
			break;
		case SL_TYPE_INT32:
			fprintf( stderr, "%.8X\n",*((uint32_t*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
			break;
		case SL_TYPE_INT16:
			fprintf( stderr, "%.8X\n",(uint32_t) *((uint16_t*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
			break;
		case SL_TYPE_INT8:
			fprintf( stderr, "%.8X\n",(uint8_t) *((uint16_t*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
			break;
		case SL_TYPE_BOOL:
			fprintf( stderr, "%s\n",( 0 != *((uint16_t*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) ) ? "True":"False" );
			break;
		case SL_TYPE_CHAR:
			fprintf( stderr, "%s\n", ((char*)( ((uint64_t ) basePtr ) + (uint64_t ) lst->offset ) ) );
			break;
		case SL_TYPE_UNIT:
			fprintf( stderr, "%s\n", lst->name.c_str() );
			break;
		default:
			fprintf( stderr,"\n");
			break;
	}
	for( unsigned i = 0; lst->nSubs > i; i++ )
	{
		STRUCT_LISTS	*ls =  & ( ( ( STRUCT_LISTS *)lst->subs)[ i ]);
		if( SL_TYPE_NONE != ls->type )
		{
			fprintf( stderr, "%s[%.4d]: %s\tCall %s\n", __FILE__,  __LINE__,indent.c_str(), lst->names[ (i * 2) + 1 ] );
			printStructList( ls, indent );
		} else {
			fprintf( stderr, "%s[%.4d]: %s\t%s is type none\n", __FILE__, __LINE__,indent.c_str(), lst->names[ (i * 2) + 1 ] );
		}
	}
}

void SCppObj::arrayDefaults( COMap *def, STRUCT_LISTS *lst, std::string indent, const char *name, sem_t *sem )
{
	indent += '\t';
	for( unsigned units = 0; lst->names[ 2 * units ]; units++ )
	{
		CppON		*oPtr;
		COString 	*sPtr;
		CppON		*dPtr;

		const char 	*c = lst->names[ (2 * units ) + 1 ];
		COMap 		*mp = (COMap *) def->findElement( c );
		if( CppON::isMap ( mp )  )
		{
			std::string typ = SCppObj_UNIT;

			STRUCT_LISTS *ls = &( (STRUCT_LISTS *) lst->subs )[ units ];

			if( CppON::isString( sPtr = (COString *) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( ! strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
			    ls->sem = openSem();
				unitDefaults( (COMap *) mp, ls, indent, c, ls->sem );
			} else if( ! strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
			    ls->sem = openSem();
				arrayDefaults( (COMap *) mp, ls, indent, c, ls->sem );
			} else {
				ls->sem = sem;
				if( ! ( dPtr = mp->findCaseElement( "defaultValue" ) )  )
				{
					throw std::invalid_argument( "Invalid configuration file.  All base classes must be provided a default value!" );
				}
				void *ptr;
				if( !strcasecmp( typ.c_str(), SCppObj_INT ) )
				{
					int sz = 10;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					switch( sz )
					{
						case 1:
							ls->offset +=  eightBitOffset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*((uint8_t *) ptr ) = (uint8_t)(dPtr->toInt() ) ;
							break;
						case 2:
							ls->offset +=  int16Offset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*((uint16_t *) ptr ) = (uint16_t)(dPtr->toInt() ) ;
							break;
						case 8:
							ls->offset +=  int64Offset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*((uint64_t *) ptr ) = (uint64_t)(dPtr->toLongInt() ) ;
							break;
						default:
						case 4:
							ls->offset +=  int32Offset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*( (uint32_t *) ptr ) = dPtr->toInt();
							break;
					}
				} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
					ls->offset +=  doubleOffset;
					ptr = (char*)basePtr + (uint64_t)ls->offset;
					*( (double *) ptr ) = dPtr->toDouble();
				} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
					ls->offset +=  charOffset;
					ptr = (char*)basePtr + (uint64_t)ls->offset;
					strcpy( (char *) ptr, dPtr->c_str() );
				} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
					ls->offset +=  eightBitOffset;
					ptr = (char*)basePtr + (uint64_t)ls->offset;
					*((uint8_t *)ptr ) = ( ( (COBoolean *) dPtr )->value() )? 0xFF : 0x00;
				}
			}
		}
	}
}

void SCppObj::unitDefaults( COMap *def, STRUCT_LISTS *lst, std::string indent, const char *name, sem_t *sem )
{
	indent += '\t';

	for( unsigned units = 0; lst->names[ 2 * units ]; units++ )
	{
		CppON		*oPtr;
		COString 	*sPtr;
		CppON		*dPtr;

		const char 	*c = lst->names[ (2 * units ) + 1 ];
		COMap 		*mp = (COMap *) def->findElement( c );
		if( CppON::isMap ( mp )  )
		{
			std::string typ = SCppObj_UNIT;

			STRUCT_LISTS *ls = &( (STRUCT_LISTS *) lst->subs )[ units ];

			if( CppON::isString( sPtr = (COString *) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( ! strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
			    ls->sem = openSem();
				unitDefaults( (COMap *) mp, ls, indent, c, ls->sem );
			} else if( ! strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
			    ls->sem = openSem();
				arrayDefaults( (COMap *) mp, ls, indent, c, ls->sem );
			} else {
				ls->sem = sem;
				if( ! ( dPtr = mp->findCaseElement( "defaultValue" ) )  )
				{
					throw std::invalid_argument( "Invalid configuration file.  All base classes must be provided a default value!" );
				}
				void *ptr;
				if( !strcasecmp( typ.c_str(), SCppObj_INT ) )
				{
					int sz = 10;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					switch( sz )
					{
						case 1:
							ls->offset +=  eightBitOffset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*((uint8_t *) ptr ) = (uint8_t)(dPtr->toInt() ) ;
							break;
						case 2:
							ls->offset +=  int16Offset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*((uint16_t *) ptr ) = (uint16_t)(dPtr->toInt() ) ;
							break;
						case 8:
							ls->offset +=  int64Offset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*((uint64_t *) ptr ) = (uint64_t)(dPtr->toLongInt() ) ;
							break;
						default:
						case 4:
							ls->offset +=  int32Offset;
							ptr = (char*)basePtr + (uint64_t)ls->offset;
							*( (uint32_t *) ptr ) = dPtr->toInt();
							break;
					}
				} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
					ls->offset +=  doubleOffset;
					ptr = (char*)basePtr + (uint64_t)ls->offset;
					*( (double *) ptr ) = dPtr->toDouble();
				} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
					ls->offset +=  charOffset;
					ptr = (char*)basePtr + (uint64_t)ls->offset;
					strcpy( (char *) ptr, dPtr->c_str() );
				} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
					ls->offset +=  eightBitOffset;
					ptr = (char*)basePtr + (uint64_t)ls->offset;
					*((uint8_t *)ptr ) = ( ( (COBoolean *) dPtr )->value() )? 0xFF : 0x00;
				}
			}
		}
	}
}

void SCppObj::listArraySems( COMap *def, STRUCT_LISTS *lst, sem_t *sem )
{
	for( unsigned units = 0; lst->names[ 2 * units ]; units++ )
	{
		const char 	*c = lst->names[ (2 * units ) + 1 ];
		fprintf( stderr, "%s[ %.4u]: %s\n", __FILE__, __LINE__, c);
	}
}

void SCppObj::listSems( COMap *def, STRUCT_LISTS *lst, sem_t *sem )
{
	for( unsigned units = 0; lst->names[ 2 * units ]; units++ )
	{
		COString 	*sPtr;
		const char 	*c = lst->names[ (2 * units ) + 1 ];
		COMap 		*mp = (COMap *) def->findElement( c );
		if( CppON::isMap ( mp )  )
		{
			std::string typ = SCppObj_UNIT;

			STRUCT_LISTS *ls = &( (STRUCT_LISTS *) lst->subs )[ units ];
			if( CppON::isString( sPtr = (COString *) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( ! strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
			    ls->sem = openSem();
				listSems( (COMap *) mp, ls, ls->sem );
			} else if( !strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
			    ls->sem = openSem();
			    listArraySems( ( COMap *) mp, ls, ls->sem );
			} else {
				ls->sem = sem;
				CppON		*oPtr;

				ls->sem = lst->sem;
				if( !strcasecmp( typ.c_str(), SCppObj_INT ) ) {
					int sz = 4;

					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					switch( sz )
					{
						case 1:
							ls->offset +=  eightBitOffset;
							break;
						case 2:
							ls->offset +=  int16Offset;
							break;
						case 8:
							ls->offset +=  int64Offset;
							break;
						default:
							ls->offset +=  int32Offset;
							break;
					}
				} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
					ls->offset +=  doubleOffset;
				} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
					ls->offset +=  charOffset;
				} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL) ) {
					ls->offset +=  eightBitOffset;
				}
			}
		}
	}
}

void SCppObj::setBasePointer( void *base, bool init, bool *initialized )
{
	bool	validInit = false;																							// Assume we are doing initialization
	basePtr = base;
	if( init )
	{
		//sem_t *initSem = (sem_t *)( (char*) basePtr + 0x20 );
		sem_t *initSem = openSem();
		/*
		 * First check to make sure it isn't already initialized
		 */
		if( 0x5A == *((char *)basePtr) )				// Being initialized
		{
			struct timespec ts;
			int				s;
			usleep( 1000 );								// Wait a millisecond for things to get moving

			if( 0 > clock_gettime( CLOCK_REALTIME, &ts ) )
			{
				ts.tv_sec = 0;
				ts.tv_nsec = 0;
			} else if( 1000000000 < ( ts.tv_nsec += 400000000 ) ) {
				ts.tv_nsec -= 1000000000;
				ts.tv_sec++;
			}

			while( ( s = sem_timedwait( initSem, &ts ) ) == -1 && errno == EINTR ) continue;
			if( 0 <= s )
			{
				sem_post( initSem );
			} else {
				fprintf( stderr, "%s[ %.4d ]: Error waiting for completion of shared memory initialization\n", __FILE__, __LINE__ );
			}
		}

		if( 0xA5 == *((char *) basePtr ) )  			// Is initialized  Check it out
		{
			uint16_t		s = 0;
			uint8_t			*out = (uint8_t *) basePtr;
			unsigned		i = 0;
			uint8_t			r;
			while( 20 > i )
			{
				r = out[ i ++ ];
				if( 0 == r || 0xFF == r )
				{
					fprintf( stderr, "\t\tFail  Zeros and 0xFF are not allowed in first twenty bytes!\n" );
					break;
				} else  {
					s += (uint16_t) r;
				}
			}
			if( 20 == i )
			{
				while( 30 > i )
				{
					r = out[ i ++ ];
					s += (uint16_t) r;
				}
			}
			if( 30 == i )
			{
				if( out[ i++ ] == (uint8_t) s )
				{
					if( out[ i ] == (uint8_t) ( s >> 8 ) )
					{
						for( i = 20, r = out[ i - 1 ]; 30 > i; i++ )
						{
							if( ++r != out[ i ] )
							{
								fprintf( stderr, "\t\tSequence not found in bytes 20 through 30!\n" );
								break;
							}
						}
						if( 30 == i )													// All checks out so we don't need to initialize anything.
						{
							validInit = true;
						}
					} else {
						fprintf( stderr, "\t\tChecksum error!\n" );
					}
				} else {
					fprintf( stderr, "\t\tChecksum error!\n" );
				}
			}
		}
		if( initialized )
		{
			*initialized = ! validInit;
		}
		/*
		 * If we are doing initialization then do it all
		 */
		if( ! validInit )
		{

			/*
			 * If we got here the we need to initialize the whole thing.
			 * Start by signaling That we are doing the initialization.
			 * Then initialize the Semaphore that is used to tell we are done
			 */
			*((char *) basePtr ) = 0x5A;
			memset( ((char*)basePtr + 0x20 ), 0, 0x10 );
			memset( ((char*)basePtr + 0x30), 0, list->size - 0x30 );

			/*
			 * Create a block of memory that can be checked to see if We are actually initialized
			 */
			struct timeval	tv;
			unsigned		i = 1;
			uint16_t		s = 0xA5;
			uint8_t			*out = (uint8_t *) basePtr;

			gettimeofday( &tv, NULL );
			srandom( tv.tv_sec );
			while( 20 > i )
			{
				uint8_t r = (uint8_t) random();
				if( 0 != r && 0xFF != r )
				{
					out[ i++ ] = r;
					s += (uint16_t) r;
				}
			}
			while( 30 > i )
			{
				out[ i ] = out[ i - 1 ] + 1;
				s += (uint16_t) out [ i++ ];
			}
			out[ i++ ] = (uint8_t) s;
			out[ i ] = (uint8_t) ( s >> 8 );

			list->sem = openSem();
			memset( (void *)( (char *)basePtr + 0x20 ), 0, doubleOffset - timeOffset );

			std::string		indent;
			for( unsigned units = 0; list->names[ 2 * units ]; units++ )
			{
				CppON		*oPtr;
				COString 	*sPtr;
				CppON		*dPtr;

				const char 	*c = list->names[ (2 * units ) + 1 ];
				COMap 		*mp = (COMap *) config->findElement( c );
				if( CppON::isMap ( mp )  )
				{
					std::string typ = SCppObj_UNIT;

					STRUCT_LISTS *ls = &( (STRUCT_LISTS *) list->subs )[ units ];

					if( CppON::isString( sPtr = (COString *) mp->findCaseElement( "type" ) ) )
					{
						typ = sPtr->c_str();
					}
					if( ! strcasecmp( typ.c_str(), SCppObj_UNIT ) )
					{
						ls->sem = openSem();
						unitDefaults( (COMap *) mp, ls, indent, c, ls->sem );
					} else if( ! strcasecmp ( typ.c_str(), SCppObj_ARRAY ) ) {
					    ls->sem = openSem();
						arrayDefaults( (COMap *) mp, ls, indent, c, ls->sem );
					} else {
						ls->sem = list->sem;
						if( ! ( dPtr = mp->findCaseElement( "defaultValue" ) )  )
						{
							throw std::invalid_argument( "Invalid configuration file.  All base classes must be provided a default value!" );
						}
						if( !strcasecmp( typ.c_str(), SCppObj_INT ) ) {
							int sz = 4;
							if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
							{
								sz = oPtr->toInt();
							}
							switch( sz )
							{
								case 1:
									ls->offset +=  eightBitOffset;
									*((uint8_t *)( (uint64_t) basePtr  + (uint64_t) ls->offset ) ) = (uint8_t)(dPtr->toInt() ) ;
									break;
								case 2:
									ls->offset +=  int16Offset;
									*((uint16_t *)( (uint64_t) basePtr  + (uint64_t) ls->offset ) ) = (uint16_t)(dPtr->toInt() ) ;
									break;
								case 8:
									ls->offset +=  int64Offset;
									*((uint64_t *)( (uint64_t) basePtr  + (uint64_t) ls->offset ) ) = dPtr->toLongInt();
									break;
								default:
									ls->offset +=  int32Offset;
									*((uint32_t *)( (uint64_t) basePtr  + (uint64_t) ls->offset ) ) = dPtr->toInt();
									break;
							}
						} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
							ls->offset +=  doubleOffset;
							*((double *) ( (uint64_t) basePtr  + (uint64_t) ls->offset ) ) = dPtr->toDouble();
						} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
							ls->offset +=  charOffset;
							strcpy( (char *) ( (uint64_t) basePtr  + (uint64_t) ls->offset ), dPtr->c_str() );
						} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
							ls->offset +=  eightBitOffset;
							*((uint8_t *)( (uint64_t) basePtr  + (uint64_t) ls->offset ) ) = ( ( (COBoolean *) dPtr )->value() )? 0xFF : 0x00;
						}
					}
				}
			}

			 *((char *) basePtr ) = 0xA5;
			sem_post( initSem );
		}
	}

	if( validInit )
	{
		list->sem = openSem();

		for( unsigned units = 0; list->names[ 2 * units ]; units++ )
		{
			COString 	*sPtr;

			const char 	*c = list->names[ (2 * units ) + 1 ];
			COMap 		*mp = (COMap *) config->findElement( c );
			if( CppON::isMap ( mp )  )
			{
				std::string typ = SCppObj_UNIT;

				STRUCT_LISTS *ls = &( (STRUCT_LISTS *) list->subs )[ units ];

				if( CppON::isString( sPtr = (COString *) mp->findCaseElement( "type" ) ) )
				{
					typ = sPtr->c_str();
				}
				if( ! strcasecmp( typ.c_str(), SCppObj_UNIT ) )
				{
				    ls->sem = openSem();
					listSems( (COMap *) mp, ls, ls->sem );
				} else if( ! strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
				    ls->sem = openSem();
				    listArraySems( (COMap *) mp, ls, ls->sem );
				} else {
					CppON		*oPtr;
					ls->sem = list->sem;
					if( !strcasecmp( typ.c_str(), SCppObj_INT ) ) {
						int sz = 4;

						if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
						{
							sz = oPtr->toInt();
						}
						switch( sz )
						{
							case 1:
								ls->offset +=  eightBitOffset;
								break;
							case 2:
								ls->offset +=  int16Offset;
								break;
							case 8:
								ls->offset +=  int64Offset;
								break;
							default:
								ls->offset +=  int32Offset;
								break;
						}
					} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
						ls->offset +=  doubleOffset;
					} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
						ls->offset +=  charOffset;
					} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
						ls->offset +=  eightBitOffset;
					}
				}
			}
		}
	}
}


sem_t *SCppObj::openSem( int idx )
{
	static 	int		semCount = 0;
			sem_t 	*sem = NULL;
			char	name[ 16 ];

	if( 0 <= idx )
	{
		semCount = idx;
	}
	sprintf( name, "/snSem_%d", semCount++ );
	if( SEM_FAILED != ( sem = sem_open( name, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1 ) ) )
	{
		sem_init( sem, 1, 1 );
	} else {
		sem = sem_open( name, O_EXCL, S_IRUSR | S_IWUSR, 1 );
	}
	sems.push_back( sem );
	return sem;
}

static bool CompareStrings( std::string a, std::string b ) { return a < b; }

void SCppObj::buildArrayNames( COMap *def, std::string indent, char **out[] )
{
	std::vector<std::string> 	s;
	std::vector<std::string> 	acro;
	unsigned					sz = 0; // 2 * def->size();
	unsigned 					j = 0;

	for( std::map<std::string, CppON *>::iterator it = def->begin(); def->end() != it; it++ )
	{
		if( CppON::isMap( it->second ) )
		{
			s.push_back( std::to_string( sz / 2 ) );
			sz += 2;
		}
	}

	char **arr = new char*[ sz + 1 ];

    for( unsigned i = 0; s.size() > i; i++)
    {
    	j = i * 2;
    	std::string &st = s.at( i );
    	arr[ j ] = new char[ st.length() + 1 ];
		strcpy( arr[ j++ ], st.c_str() );
    	arr[ j ] = new char[ st.length() + 1 ];
		strcpy( arr[ j++ ], st.c_str() );
    }

    arr[ j ] = NULL;
    *out = arr;
}

void SCppObj::buildNames( COMap *def, std::string indent, char **out[]  )
{
	std::vector<std::string> 	s;
	std::vector<std::string> 	acro;
	unsigned					sz = 0; // 2 * def->size();

	/*
	 * Find out how many names/(objects + arrays) we have
	 */
	for( std::map<std::string, CppON *>::iterator it = def->begin(); def->end() != it; it++ )
	{
		if( ( CppON::isMap( it->second )  && it->first.compare( "update" ) ) || ! it->first.compare( "threeAxis" ) )
		{
			sz += 2;
			s.push_back( it->first );
		} else if( CppON::isArray( it->second ) ) {
			sz += 2;
			s.push_back( it->first );
		}
	}

	/*
	 * Put them in alphabetical order
	 */
    std::sort( s.begin(), s.end(), CompareStrings );

    /*
     * Create room to store search string and full names
     */
	char **arr = new char*[ sz + 1 ];

    for( unsigned i = 0; s.size() > i; i++)
    {
    	const char *sPtr = s.at( i ).c_str();
		std::string			req("");
		unsigned 			j;
		if( 0 == i )
		{
			if( 1 < s.size() )
			{
				const char *s1Ptr = s.at( i + 1 ).c_str();
				for( j = 0; sPtr[ j ] == s1Ptr[ j ]; j++ )									// for all characters equal to the next line;
				{
					req += sPtr[ j ];
				}
				req += sPtr[ j ];
			} else {
				req += *sPtr;
				acro.push_back( req );
				break;
			}

		} else if( s.size() == ( i + 1 ) ) {
    		const char *s1Ptr = s.at( i - 1 ).c_str();
			for( j = 0; sPtr[ j ] == s1Ptr[ j ]; j++ )									// for all characters equal to the line before this one
			{
				req += sPtr[ j ];
			}
			req += sPtr[ j ];
		} else {
    		const char *s1Ptr = s.at( i - 1 ).c_str();
    		const char *s2Ptr = s.at( i + 1 ).c_str();
			for( j = 0; (strlen( sPtr ) > j ) && ( ( strlen( s1Ptr ) > j && sPtr[ j ] == s1Ptr[ j ] ) || ( strlen( s2Ptr ) > j && sPtr[ j ] == s2Ptr[ j ] ) ); j++ )		// For all characters equal to the next or previous line;
			{
				req += sPtr[ j ];
			}
			req += sPtr[ j ];
		}
		acro.push_back( req );
    }
	unsigned i;
	std::vector<std::string>::iterator its = s.begin();
	std::vector<std::string>::iterator ita = acro.begin();
	for( i = 0; sz > i; its++, ita++ )
	{
		arr[ i ] = new char[ ita->length() + 1 ];
		strcpy( arr[ i++ ], ita->c_str() );
		arr[ i ] = new char[ its->length() + 1 ];
		strcpy( arr[ i++ ], its->c_str() );
	}
	arr[ i ] = NULL;
	*out = arr;
}

uint32_t SCppObj::buildArray( COMap *def, STRUCT_LISTS *unit, std::string indent, const char *name )
{
	COString	*sPtr;
	COMap	*mp;

	int units = 0;
	int	names = 0;
	indent += '\t';

	/*
	 * Figure out how many units the base has in it then use that to allocate the structure
	 */
	for( std::map<std::string, CppON *>::iterator it = def->begin(); def->end() != it; it++ )
	{
		if( CppON::isMap( it->second ) )
		{
			std::string typ = "";
			mp = (COMap *) it->second;
			if( CppON::isString( sPtr = (COString*) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( !strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
				units++;
				names++;
			} else if( typ.length() ) {
				names++;
			}
		}
	}
	unit->subs = new STRUCT_LISTS[ unit->nSubs = names ];
	unit->type = SL_TYPE_ARRAY;
	unit->size = 0;
	unit->name = name;
	unit->names = NULL;

	buildArrayNames( def, indent, &( unit->names )  );

	for( units = 0; unit->names[ 2 * units ]; units++ )
	{
		COString 	*sPtr;
		CppON		*oPtr;
		const char 	*c = unit->names[ (2 * units ) + 1 ];
		COMap 		*mp = (COMap *) def->findElement( c );
		if( CppON::isMap ( mp )  )
		{
			std::string typ = "";
			STRUCT_LISTS *ls = &( (STRUCT_LISTS *) unit->subs )[ units ];
			ls->names = NULL;
			ls->nSubs = 0;
			ls->def = mp;

			if( CppON::isString( sPtr = (COString*) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( !strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
				ls->time = 0;
				unit->size += buildUnit( mp, ls, indent, c );
			} else if ( ! strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
				ls->time = 0;
				unit->size += buildArray( mp, ls, indent, c );
			} else {
				ls->name = c;
				ls->time = timeOffset;
				timeOffset += sizeof( uint64_t );

				if( !strcasecmp( typ.c_str(), SCppObj_INT ) )
				{
					int sz = 4;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					switch( sz )
					{
						case 1:
							ls->type = SL_TYPE_INT8;;
							ls->offset = (uint32_t) eightBitOffset;
							ls->size = sizeof( uint8_t );
							eightBitOffset += ls->size;
							unit->size += ls->size;
							break;
						case 2:
							ls->type = SL_TYPE_INT16;
							ls->offset = (uint32_t) int16Offset;
							ls->size = sizeof( uint16_t );
							int16Offset += ls->size;
							unit->size += ls->size;
							break;
						case 8:
							ls->type = SL_TYPE_INT64;
							ls->offset = (uint32_t) int64Offset;
							ls->size = sizeof( uint64_t );
							int64Offset += ls->size;
							unit->size += ls->size;
							break;
						default:
							ls->type = SL_TYPE_INT32;
							ls->offset  = (uint32_t) int32Offset;
							ls->size = sizeof( uint32_t );
							int32Offset += ls->size;
							unit->size += ls->size;
							break;
					}
				} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
					ls->type = SL_TYPE_DOUBLE;
					ls->offset = (uint32_t) doubleOffset;
					ls->size = sizeof( double );
					unit->size += ls->size;
					doubleOffset += ls->size;
				} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
					int sz = 16;
					ls->type = SL_TYPE_CHAR;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					ls->offset = (uint32_t) charOffset;
					ls->size = sz;
					unit->size += sz;
					charOffset += sz;
				} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
					ls->type = SL_TYPE_BOOL;
					ls->offset  = (uint32_t) eightBitOffset;
					unit->size++;
					ls->size = 1;
					eightBitOffset++;
				}
			}
		} else {
			fprintf( stderr, "%s[ %.4u ]: is type %d\n", __FILE__, __LINE__, mp->type() );
		}
	}

	unit->size = eightBitOffset + charOffset + doubleOffset + int32Offset + int64Offset + int16Offset;
	return unit->size;
}

uint32_t SCppObj::buildUnit( COMap *def, STRUCT_LISTS *unit, std::string indent, const char *name )
{
	COString	*sPtr;
	COMap	*mp;

	int units = 0;
	int	names = 0;
	indent += '\t';

	/*
	 * Figure out how many units the base has in it then use that to allocate the structure
	 */
	for( std::map<std::string, CppON *>::iterator it = def->begin(); def->end() != it; it++ )
	{
		if( CppON::isMap( it->second) )
		{
			std::string typ = "";
			mp = (COMap *) it->second;
			if( CppON::isString( sPtr = (COString*) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( !strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
				units++;
				names++;
			} else if( typ.length() ) {
				names++;
			}
		}
	}

	unit->subs = new STRUCT_LISTS[ unit->nSubs = names ];
	unit->type = SL_TYPE_UNIT;
	unit->size = 0;
	unit->name = name;
	unit->names = NULL;

	buildNames( def, indent, &(unit->names)  );

	for( units = 0; unit->names[ 2 * units ]; units++ )
	{
		COString 	*sPtr;
		CppON		*oPtr;
		const char 	*c = unit->names[ (2 * units ) + 1 ];
		COMap 		*mp = (COMap *) def->findElement( c );

		if( CppON::isMap ( mp )  )
		{
			std::string typ = "";
			STRUCT_LISTS *ls = &( (STRUCT_LISTS *) unit->subs )[ units ];
			ls->names = NULL;
			ls->nSubs = 0;
			ls->def = mp;

			if( CppON::isString( sPtr = (COString*) mp->findCaseElement( "type" ) ) )
			{
				typ = sPtr->c_str();
			}
			if( !strcasecmp( typ.c_str(), SCppObj_UNIT ) )
			{
				ls->time = 0;
				unit->size += buildUnit( mp, ls, indent, c );
			} else if ( ! strcasecmp( typ.c_str(), SCppObj_ARRAY ) ) {
				ls->time = 0;
				unit->size += buildArray( mp, ls, indent, c );
			} else {
				ls->name = c;
				ls->time = timeOffset;
				timeOffset += sizeof( uint64_t );

				if( !strcasecmp( typ.c_str(), SCppObj_INT ) )
				{
					int sz = 4;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					switch( sz )
					{
						case 1:
							ls->type = SL_TYPE_INT8;;
							ls->offset = (uint32_t) eightBitOffset;
							ls->size = sizeof( uint8_t );
							eightBitOffset += ls->size;
							unit->size += ls->size;
							break;
						case 2:
							ls->type = SL_TYPE_INT16;
							ls->offset = (uint32_t) int16Offset;
							ls->size = sizeof( uint16_t );
							int16Offset += ls->size;
							unit->size += ls->size;
							break;
						case 8:
							ls->type = SL_TYPE_INT64;
							ls->offset = (uint32_t) int64Offset;
							ls->size = sizeof( uint64_t );
							int64Offset += ls->size;
							unit->size += ls->size;
							break;
						default:
							ls->type = SL_TYPE_INT32;
							ls->offset  = (uint32_t) int32Offset;
							ls->size = sizeof( uint32_t );
							int32Offset += ls->size;
							unit->size += ls->size;
							break;
					}
				} else if( !strcasecmp( typ.c_str(), SCppObj_FLOAT ) ) {
					ls->type = SL_TYPE_DOUBLE;
					ls->offset = (uint32_t) doubleOffset;
					ls->size = sizeof( double );
					unit->size += ls->size;
					doubleOffset += ls->size;
				} else if( !strcasecmp( typ.c_str(), SCppObj_STRING ) ) {
					int sz = 16;
					ls->type = SL_TYPE_CHAR;
					if( CppON::isNumber( oPtr = mp->findCaseElement( "size" ) ) )
					{
						sz = oPtr->toInt();
					}
					ls->offset = (uint32_t) charOffset;
					ls->size = sz;
					unit->size += sz;
					charOffset += sz;
				} else if( !strcasecmp( typ.c_str(), SCppObj_BOOL ) ) {
					ls->type = SL_TYPE_BOOL;
					ls->offset  = (uint32_t) eightBitOffset;
					unit->size++;
					ls->size = 1;
					eightBitOffset++;
				}
			}
		}
	}
	unit->size = eightBitOffset + charOffset + doubleOffset + int32Offset + int64Offset + int16Offset;
	return unit->size;
}

