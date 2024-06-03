/*
 *
 *   File Name: CppON.hpp
 *  Created on: 10 Jan 2010 - 26 May 2024
 *      Author: Jeff Hertig
 * Description: This is a library used to implement the concept of json, xml and tnetstrings in C++ as a hierarchical class structure.
 *              It is a very powerful and useful library in that it converts different string representations of data structures to and from
 *              the class library.  This enables easy creation and manipulation of data in c++ and easy conversion back in forth to string
 *              formats for storage and transmission over a serial link.
 *              Class structure can be created by loading character data directly from files or from C-strings or std::strings. Or they can be
 *              constructed completely through C code using the new operator.
 *
 *              It should be noted, that once created and added into the hierarchy, the root node "owns" all the branches of the hierarchy and will
 *              destroy them when destroyed.  This means you should be careful to not add a stack created item to a hierarchy as it will be destroyed
 *              when the code loses scope and also when the hierarchy is destroyed.
 *
 *              The route of most "trees" is usually one of the two "container" classes: the Map object or an Array.  Usually a Map.
 *              A Map represents a hashmap and is actually implemented as an extension of a std::map< std::string, CppON *> container.  It is
 *              represented as a map in JSON and TNetStrings as well as an object in JavaScript.  In fact the method value() returns a pointer
 *              to the internal map.  Care should be used not to modify the contents returned from this function.
 *              An Array represents a dynamic list of objects and is implemented as extension of a std::vector< CppON *> container internally. Like
 *              the Map the value() method will return a pointer to the internal list.
 *
 *              The Container classes, COArray and COMap can hold other CppON's including other Containers.  In this way very complex structures can
 *              be built and accessed internally.  To navigate the hierarchy a forward slash "/" can be inserted to delimit parts of a name.
 *
 *              In addition to the two container classes there are five data classes: COBoolean, CODouble, COInteger, COString and CONull which are all
 *              represented in JSON.  The name implies the data type.  The "Do" on the beginning of each type denotes "Data Object" of the base class
 *              CppON.
 *
 *              The CppON is the base class of a number of sub-classes to represent the elements that can be access as a dynamic structure.
 *              The C++ program can create, read, modify and otherwise manipulate the data easily.  It can then save the data as a string in a file or
 *              transmit it through a queue or socket to another application. Conversion back and forth between object structure and strings is executed
 *              with a single call.
 *
 *              Supported string types are json, tnetstrings and xml(if xml2 library is available)
 *
 *              The base class is CppON and is meant to be pure virtual class while the following classes are extended from it:
 *              COArray       -  Container class: Implemented as a stl vector it is basically an array container which
 *                              can hold a list of objects including other COArrays.
 *              COMap         -  Container class: Implements a hash_map type container of key:value pares where the
 *                              value can be any of the possible objects.  Keys must be strings but
 *                              values can be any of these objects including other COMaps.
 *
 *              COBoolean     -  This represents a boolean value of true or false.
 *              CODouble      -  This represents a floating point number
 *              COInteger     -  This represents an integer number but can be a char, short, int, long
 *                              or long long.
 *              COString      -  This represents a character string it acts much like the std::string object
 *              CONull        -  represents a undefined value.
 *
 *   examples:
 *              To create a Map object from a string:
 *                COMap       myMap( "{\"command\":\"myCommand\", \"reason\":\"init\", \"enable\":true, \"param\":{\"one\":1,\"two\":2.0,\"arr\":[1,2,3,4.0]}}");
 *                will create a structure much like:
 *                  {                                           - A Map container object
 *                    command:"myCommand",                      - A string called command with contents "myCommand"
 *                    reason:"init",                            - A string called reason with contents "init"
 *                    param: {                                  - A Map container object that holds one, two and arr.
 *                      one: 1,                                 - An Integer named "one" containing the integer value "1"
 *                      two: 2.0,                               - A Double named "two" containing the double value "2.0"
 *                      arr: [                                  - An array object named "arr" containing the integers 1,2,3 and the double 4.0
 *                        1,
 *                        2,
 *                        3,
 *                        4.0
 *                      ]
 *                    }
 *                  }
 *              There are a number of ways to access the data but the typical one for a map would be to use the findElement method as below:
 *                COString *sObj = (COString *) myMap.findElement( "command" )
 *              Which would return the Object called "command".
 *
 *              A simple test can be used to verify that the value returned is valid by using one of the static functions of CppON: isString(), isDouble,
 *              isBoolean, isNumber(), isMap() or isArray().
 *
 *              In this case CppON::isString( sObj ); would return true, while CppON::isMap( sObj ) would return false;
 *              Likewise if the function were used on a value not included it would return false. I.E. if( CppON::isString( myMap.findElement( "cmd" ) ) ) would
 *              fail because "cmd" is not in myMap.
 *
 *              You could also use CODouble dObj = (CODouble*) myMap.findCaseElement( "param/two" ); to return "two" of the "param" object.
 *
 *              You could modify the value of two to 20.2
 *                *((CODouble *) myMap.findElement( "param/two" ) = 20.2;
 *
 *              You could add a object to the class by doing something like this:
 *                COMap *param;
 *                if( CppON::isMap( param = (COMap*) myMap->findElement( "param" ) ) )
 *                  param->append( "added", new COInteger( 3 ) );
 *
 *              Afterwards you could convert it to a string by:
 *                std::string *str = myMap.toJsonString( myMap );
 *
 *              str would then contain "{\"command\":\"myCommand\","reason\":\"init\",\"param\":{\"one\":1,\"two\":20.2,\"arr\":[1,2,3,4.0],\"added\":3}}"
 *
 *              You could create the whole string programmatically in C by doing the following
 *              {
 *                COMap       myMap;
 *                COMap       *param;
 *                COArray     *arr;
 *                std::string *str;
 *
 *                // construct the map
 *
 *                myMap.append( "command", new COString( "myCommand" ) );
 *                myMap.append( "reason", new COString( "init" );
 *                myMap.append( "param", param = new COMap() );
 *                param->append( "one", new COInteger( 1 ) );
 *                param->append( "two", new CODouble( 2.0 ) );
 *                param->append( "arr", arr = new COArray() );
 *                arr->append( new COInteger( 1  ) );
 *                arr->append( new COInteger( 2  ) );
 *                arr->append( new COInteger( 3  ) );
 *                arr->append( new CODouble( 4.0  ) );
 *
 *                // Convert it to a JSON string
 *
 *                str = myMap.toJsonString();
 *
 *                // str is a pointer to a Json string:
 *                // "{\"command\":\"myCommand\", \"reason\":\"init\",\"param\":{\"one\":1,\"two\":2.0,\"arr\":[1,2,3,4.0]}}"
 *
 *                delete( str );  // String returned from the toJsonString() function must be deleted.
 *
 *                You could now do something like:
 *
 *                COString *sTmp = (COString*) myMap.findElement( "reason" ) or myMap.findCaseElement( "ReaSon" ) to get the COString element reason.
 *                // We must cast the returned value to the type we expect it to be because the "getter" functions return just a base class.
 *                *sTmp = "initialization"; // would then change it to "initialization"
 *
 *                // However that is dangerous because the structure might not have a "reason" element or the element might be of the wrong type in it.
 *                // So the following would be better:
 *                COInteger *sTmp;
 *                if( CppON::isString( sTmp = (COString *) myMap.findCaseElement("reason") ) )
 *                   sTmp = "initialization";
 *                // In this case we search for the element, casting it to the string type.  But then we use the Static base class funciton to make sure it
 *                // is not NULL and that it is of "COString" type.)
 *
 *                Similarly we could do something like:
 *                COArray *arr;
 *                CppON *item;
 *
 *                if( CppON::isArray( arr = (COArray *)myMap.findElement( "arr" ) ) )
 *                {
 *                   if( CppON::isInteger( item = arr.at( 2 ) ) )
 *                      *((COInteger *) item) = 6;
 *                   else if( CppON::isDouble( item ) )
 *                      *((CODouble *) item) = 6.0;
 *
 *                   // However, because both COInteger and CODouble are numbers this could have been accomplished by
 *                   if( CppON::isNumber( item = arr.at( 2 ) )
 *                     *item = 6;
 *                }
 *
 *                // You now could do a "dump" of the structure to a file like this:
 *                myMap.dump( stdout );
 *                // and get this
 *                {
 *                   command: "myCommand",
 *                   reason: "Initialization",
 *                   arr: [
 *                     1,
 *                     2,
 *                     6,
 *                     4.0
 *                   ]
 *                }
 *
 *                //  or of course convert it to a JSON or TNetString;
 *
 *              }  // on going out of scope, the the myMap structure will be destroyed and all the memory reclaimed.
 *                 // However if you had do it differently and done:
 *                 COMap   *myMap = new COMap();
 *                 // You would need to do a:
 *                 delete myMap;
 *
 *
 *
 *
 *    Depends:  This Library depends on STL:map, STL:string, STL: vector and the "jansson" library.
 *    AND, the xml2 if XML support is desired. In which case you will want to set the "HAS_XML" flag to true;
 */

#ifndef CPPON_HPP_
#define CPPON_HPP_

#define HAS_XML 0
#define SIXTY_FOUR_BIT 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <semaphore.h>

#include <jansson.h>


#if HAS_XML
#include <libxml/xmlversion.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#endif

#define  UD_DOUBLE -999999999.123

extern bool simulatorMode;


/*
 * Structure used to tell what data type is being represented.
 */
enum CppONType
{
    UNKNOWN_CPPON_OBJ_TYPE,
    INTEGER_CPPON_OBJ_TYPE,
    DOUBLE_CPPON_OBJ_TYPE,
    STRING_CPPON_OBJ_TYPE,
    NULL_CPPON_OBJ_TYPE,
    BOOLEAN_CPPON_OBJ_TYPE,
    MAP_CPPON_OBJ_TYPE,
    ARRAY_CPPON_OBJ_TYPE
};

enum CppONOperator
{
	CPPON_ADD,
	CPPON_SUBTRACT,
	CPPON_MULTIPLY,
	CPPON_DIVIDE
};

/*
 * This is the base class.  It is not meant to be instantiated directly.
 * However you can use the "factory" to create a copy of it if you don't know its derived class
 *
 * Generally it is used to access base classes without taking the trouble to determine their type
 * which can be exposed using the "typ" variable or type() method.  which will return and object of "CppONType" enumeration type
 * or the either the static or non static methods can be used: isNumber(), isInteger(), isDouble(), isBoolean(), isMap(), isArray()
 * other functions can be used without knowing the type which will result in it using its "best guess" like:
 *  toInt() converts it to an integer to give it up
 *  toLongInt() converts it to a long integer
 *  toDouble() converts it to a double
 *  c_str() converts it to a string
 * various operators and methods are available for comparing two objects
 *   diff will attempt to compare to like objects and create a object representing their differences
 *   == or != can be used to get a boolean value of whether they contain the same information
 *
 *   then there are a number of functions to create a data object from a string:
 *     parse( const char *str, char **rstr );       // Create a CppON object from a net string
 *     parseJson( const char *str );                // Create a CppON object form a json string
 *     parseJson( json_t *ob, std::string &tabs );  // Create a CppON object form a Json object
 *     parseXML( const char *str );
 *     parseCSV(const char *str );                  // parse a CSV file into  and array of arrays;
 *     parseTSV(const char *str );                  // parse a TSV file into  and array of arrays;
 *     parseJsonFile( const char *path );           // Read a file and create a CppON from it.
 *
 * toNetString( const char *str, char styp );  can be used to create a TNet String from the data
 * dump( FILE *fp); can be used to write the whole contents to a file
 *
 * Other methods are available on the individual container classes and object classes to access and minimulate the data
 * As stated the root class is just there for accessing and moving the objects in a generic sense.
 *
 */
class CppON
{
public:
											CppON( CppON &jt );
											CppON(){ data = NULL; typ=UNKNOWN_CPPON_OBJ_TYPE; siz = 0; precision=-1; }
											CppON( CppONType typ=UNKNOWN_CPPON_OBJ_TYPE );
											CppON( CppON *jt = NULL );
	virtual									~CppON();
	static	CppON							*factory( CppON &jt );
	static	CppON							*factory( CppON *jt ) { return factory( *jt ); }
			CppONType						type(){ return typ;}
    virtual int								size(){ return siz;}
    virtual void							dump( FILE *fp = stderr );
    virtual void							cdump( FILE *fp = stderr );
    virtual std::string						*toCompactJsonString();
			void							*getData(){ return data; }
			double							toDouble(void);
			long long						toLongInt(void);
			int								toInt(void);
			bool							toBoolean(void);
			bool							isNumber(void){ return (DOUBLE_CPPON_OBJ_TYPE==typ || INTEGER_CPPON_OBJ_TYPE==typ || BOOLEAN_CPPON_OBJ_TYPE==typ); }
			bool							isMap() { return MAP_CPPON_OBJ_TYPE == typ; }
			bool							isArray() { return ARRAY_CPPON_OBJ_TYPE == typ; }
			bool							isString() { return STRING_CPPON_OBJ_TYPE == typ; }
			bool							isBoolean() { return BOOLEAN_CPPON_OBJ_TYPE == typ; }
			bool							isInteger() { return INTEGER_CPPON_OBJ_TYPE == typ; }
			bool							isDouble() { return DOUBLE_CPPON_OBJ_TYPE == typ; }
			bool							operator == ( CppON &newObj );
                                            // cppcheck-suppress constParameter
			bool							operator == ( CppON *newObj ){ return( *this == *newObj );}
                                            // cppcheck-suppress constParameter
			bool							operator != ( CppON &newObj ){ return( !( *this == newObj ) );}
                                            // cppcheck-suppress constParameter
			bool							operator != ( CppON *newObj ){ return( !( *this == *newObj ) );}
			virtual							CppON *operator = ( CppON &val );
                                            // cppcheck-suppress constParameter
			CppON							*diff( CppON &newObj, const char *name = NULL );
	const	char							*c_str( );
	static	bool							isNumber( CppON *val ) { return ( val && ( DOUBLE_CPPON_OBJ_TYPE==val->typ || INTEGER_CPPON_OBJ_TYPE==val->typ || BOOLEAN_CPPON_OBJ_TYPE==val->typ ) ); }
	static  bool							isMap( CppON *val ) { return ( val && MAP_CPPON_OBJ_TYPE == val->typ ); }
	static  bool							isArray( CppON *val ) { return ( val && ARRAY_CPPON_OBJ_TYPE == val->typ ); }
	static  bool							isString( CppON *val ) { return ( val && STRING_CPPON_OBJ_TYPE == val->typ ); }
	static  bool							isBoolean( CppON *val ) { return ( val && BOOLEAN_CPPON_OBJ_TYPE == val->typ ); }
	static  bool							isInteger( CppON *val ) { return ( val && INTEGER_CPPON_OBJ_TYPE == val->typ ); }
	static  bool							isDouble( CppON *val ) { return ( val && DOUBLE_CPPON_OBJ_TYPE == val->typ ); }
	static  bool							isObj( CppON *val ){ return ( val && INTEGER_CPPON_OBJ_TYPE <= val->typ && ARRAY_CPPON_OBJ_TYPE >= val->typ ); }

	static	CppON							*readObj( FILE *fp );
	static  CppON							*parse( const char *str, char **rstr );         // Create a CppON object from a net string
	static  CppON							*parseJson( const char *str );                  // Create a CppON object form a json string
	static  CppON							*parseJson( json_t *ob, std::string &tabs );    // Create a CppON object form a Json object
#if HAS_XML
	static  CppON							*parseXML( const char *str );
#endif
	static  CppON							*parseCSV(const char *str );                    // parse a CSV file into  and array of arrays;
	static  CppON							*parseTSV(const char *str );                    // parse a TSV file into  and array of arrays;
	static  CppON							*parseJsonFile( const char *path );             // Read a file and create a CppON from it.
	static  CppON							*guessDataType( const char *str );
	static  unsigned char					*findTNetStringArg( const char *arg, int argSize, const char *str, const char **next = NULL, int *cnt = NULL );
private:
			void							deleteData();
protected:
	static	std::string						*toNetString( const char *str, char styp );

			void							*data;											// This is an allocated pointer to the data
			CppONType						typ;											// This is used to indicate the object type
			int								siz;											// Either the size of the object as in 1,2 4, 8 bytes
			std::string						str;											// Used when c_str called;
                                															// or the number of elements in the list.
			std::vector<std::string>		order;											// only used for Map.  Order in which keys appear
			char							precision;										// precision to be used for double numbers
};

/*
 * The Integer is a basic integer value class it will actually allocate different data size base on the type used to construct it
 *
 * This is one time used to represent all integer type values.  It may have been easier to make multiple
 * types of integers sub classed from a single one but I chose to do it this way.
 *
 */
//template<typename T>

class COInteger : public CppON
{
public:
											COInteger( COInteger &it );
											COInteger( COInteger *it = NULL );
											COInteger( char i ='\0' ) : CppON( INTEGER_CPPON_OBJ_TYPE ) { unSigned = false;data = new ( char ); *((char*) data ) = i; siz = sizeof( char);}
											COInteger( int16_t i = 0 ) : CppON( INTEGER_CPPON_OBJ_TYPE ) { unSigned = false;data = new ( int16_t ); *((int16_t*) data) = i; siz = sizeof( int16_t );}
											COInteger( int32_t i = 0 ) : CppON( INTEGER_CPPON_OBJ_TYPE ) { unSigned = false;data = new (int32_t); *((int32_t*) data) = i; siz = sizeof(int32_t);}
											COInteger( uint32_t i = 0 ) : CppON( INTEGER_CPPON_OBJ_TYPE ) { unSigned = true; data = new (uint32_t); *((uint32_t*) data) = i; siz = sizeof(uint32_t);}
											COInteger( int64_t i = 0) : CppON( INTEGER_CPPON_OBJ_TYPE ) { unSigned = false;data = new(int64_t); *((int64_t *) data ) = i; siz = sizeof(int64_t); }
											COInteger( uint64_t i = 0) : CppON( INTEGER_CPPON_OBJ_TYPE ) { unSigned = true;data = new(uint64_t); *((uint64_t *) data ) = i; siz = sizeof(uint64_t); }
			COInteger						*operator=(COInteger &val );
											// cppcheck-suppress constParameter
			COInteger						*operator=(COInteger *val ) { return( *this = *val );}
			bool							operator == ( COInteger &newObj );
											// cppcheck-suppress constParameter
			bool							operator == ( COInteger *newObj ){ return( *this == *newObj ); }
											// cppcheck-suppress constParameter
			bool							operator != ( COInteger &newObj ) { return( ! ( *this == newObj ) );}
											// cppcheck-suppress constParameter
			bool							operator != ( COInteger *newObj ){ return( ! ( *this == *newObj ) );}

template<typename T> T                      operator = (const T t ) { if( data ) delete( ( T* ) data ); data = new ( T ); *(( T *) data ) = t; siz = sizeof( T ); return t; }
//			int								operator=( char i ){ if(data) delete( (int*)data ); data = new(char); *((char*) data ) = i; siz = sizeof( char ); return( i ); }
//			int								operator=( short i ){ if(data) delete( (int*)data ); data = new(short); *((short*) data ) = i; siz = sizeof( short ); return( i ); }
//			int								operator=( long i ){ if(data) delete( (int*)data ); data = new(long); *((long*) data ) = i; siz = sizeof( long ); return( i ); }
<<<<<<< HEAD
//			int32_t							operator=( int32_t i ){ if(data) delete( (int32_t*)data ); data = new(int32_t); *((int32_t*) data ) = i; siz = sizeof( int32_t ); return( i ); }
=======
//			int								operator=( int i ){ if(data) delete( (int*)data ); data = new(int); *((int*) data ) = i; siz = sizeof( int ); return( i ); }
>>>>>>> 63be8b7697ec4bd9b60a94d73e998684583c8fcc
//			long long						operator=(long long i){ if(data) delete((long long*)data);data=new(long long); *((long long*) data ) = i; siz = sizeof( long long ); return( i ); }

template<typename T> T						operator += ( const T t){ return (int64_t) doOperation( (uint64_t) t, CPPON_ADD ); }
template<typename T> T						operator -= ( const T t){ return (int64_t) doOperation( (uint64_t) t, CPPON_SUBTRACT ); }
template<typename T> T						operator *= ( const T t){ return (int64_t) doOperation( (uint64_t) t, CPPON_MULTIPLY ); }
template<typename T> T						operator /= ( const T t){ return (int64_t) doOperation( (uint64_t) t, CPPON_DIVIDE ); }
<<<<<<< HEAD

//			int64_t							operator += (int64_t val ){ return (int64_t) doOperation( (uint64_t) val, CPPON_ADD ); }
//			int64_t							operator -= (int64_t val ){ return (int64_t) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			int64_t							operator *= (int64_t val ){ return (int64_t) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			int64_t							operator /= (int64_t val ){ return (int64_t) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
=======
//			long long						operator += (long long val ){ return (long long) doOperation( (uint64_t) val, CPPON_ADD ); }
//			long long						operator -= (long long val ){ return (long long) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			long long						operator *= (long long val ){ return (long long) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			long long						operator /= (long long val ){ return (long long) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
//			uint64_t						operator += (uint64_t val ){ return doOperation( (uint64_t) val, CPPON_ADD ); }
//			uint64_t						operator -= (uint64_t val ){ return doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			uint64_t						operator *= (uint64_t val ){ return  doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			uint64_t						operator /= (uint64_t val ){ return doOperation( (uint64_t) val, CPPON_DIVIDE ); }
>>>>>>> 63be8b7697ec4bd9b60a94d73e998684583c8fcc
//			int32_t 						operator += (int32_t val ){ return (int32_t) doOperation( (uint64_t) val, CPPON_ADD ); }
//			int32_t							operator -= (int32_t val ){ return (int32_t) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			int32_t							operator *= (int32_t val ){ return (int32_t) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			int32_t							operator /= (int32_t val ){ return (int32_t) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
<<<<<<< HEAD
//			char 							operator += (char val ){ return (char) doOperation( (uint64_t) val, CPPON_ADD ); }
//			char							operator -= (char val ){ return (char) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			char							operator *= (char val ){ return (char) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			char							operator /= (char val ){ return (char) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
//			short 							operator += (short val ){ return (short) doOperation( (uint64_t) val, CPPON_ADD ); }
//			short							operator -= (short val ){ return (short) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			short							operator *= (short val ){ return (short) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			short							operator /= (short val ){ return (short) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
=======
//			uint32_t 						operator += (uint32_t val ){ return (uint32_t) doOperation( (uint64_t) val, CPPON_ADD ); }
//			uint32_t						operator -= (uint32_t val ){ return (uint32_t) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
//			uint32_t						operator *= (uint32_t val ){ return (uint32_t) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
//			uint32_t						operator /= (uint32_t val ){ return (uint32_t) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
			char 							operator += (char val ){ return (char) doOperation( (uint64_t) val, CPPON_ADD ); }
			char							operator -= (char val ){ return (char) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
			char							operator *= (char val ){ return (char) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
			char							operator /= (char val ){ return (char) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
			short 							operator += (short val ){ return (short) doOperation( (uint64_t) val, CPPON_ADD ); }
			short							operator -= (short val ){ return (short) doOperation( (uint64_t) val, CPPON_SUBTRACT ); }
			short							operator *= (short val ){ return (short) doOperation( (uint64_t) val, CPPON_MULTIPLY ); }
			short							operator /= (short val ){ return (short) doOperation( (uint64_t) val, CPPON_DIVIDE ); }
>>>>>>> 63be8b7697ec4bd9b60a94d73e998684583c8fcc

			int								size() override { return ( data ) ? siz : 0;}               // return the 1 if it is defined
			int64_t							longValue();                                                // get the long long value of the object
			char							charValue(){ return (data) ? *((char *)data) : 0; }         // get the long long value of the object
			short							shortValue(){ return (data) ? *((short *)data) : 0; }       // get the long long value of the object
			int32_t							intValue(){ return (data) ? *((int32_t *)data) : 0; }           // get the long long value of the object
			std::string						*toNetString();                                             // convert to net string format
			std::string						*toJsonString();                                            // convert to json string format
			void							dump( FILE *fp = stderr ) override ;
			void							cdump( FILE *fp = stderr ) override ;
			const char						*c_str();
			CppON							*diff( CppON &newObj );
private:

			bool							unSigned;
			uint64_t						doOperation( uint64_t val, CppONOperator op );
};

/*
 * The Data Object Double class, CODouble encapsulates the functionality of a floating point value in JSON.
 * Besides the usual conversion and comparison functions it has one extension tht can be helpful:
 *    The Precision value. This is not set by default but if it is set then the stored value will be rounded to the given
 *    number of decimal places.  What's more the value will have a built in hysteresis of +-3/4 a significant digit.  I.E. if
 *    the object has a precision of 2 and the value is currently 3.12, setting it to 3.1274 will not change the value but 3.1275
 *    will change it to 3.13.  Note that setting it to 3.135 will round it up to 3.14 though. This is because once the hysteresis
 *    condition is met, then it will round to the nearest precision.
 *
 * It should be noted that in this version of the library all values are stored as doubles.
 */

class CODouble : public CppON
{
public:
											CODouble( CODouble &dt );
											CODouble( CODouble *dt = NULL );
											CODouble( double d = 0.0 ) : CppON( DOUBLE_CPPON_OBJ_TYPE ) { precision=10; data = new (double); *((double*) data) = d; siz = sizeof(double);}
			unsigned char                	Precision() { return precision; }
			unsigned char					Precision( unsigned char p ){ precision = p; return precision;}
			bool							operator == ( CODouble &newObj ) { return(  *( (double *)newObj.data) == *( (double *) data) ); }
											// cppcheck-suppress constParameter
			bool							operator == ( CODouble *newObj ) { return(   ( *this == *newObj ) ); }
											// cppcheck-suppress constParameter
			bool							operator != ( CODouble &newObj ) { return( ! ( *this ==  newObj ) ); }
											// cppcheck-suppress constParameter
			bool							operator != ( CODouble *newObj ) { return( ! ( *this == *newObj ) ); }
			double							operator = ( const double& val );
			CODouble						*operator = ( CODouble &val );
											// cppcheck-suppress constParameter
			CODouble						*operator = ( CODouble *val ) { return( *this = *val ); }
			double							operator += ( double val ) { if( data ) { *(( double *) data ) += val; return *((double *) data);} return UD_DOUBLE;}
			double							operator -= ( double val ) { if( data ) { *(( double *) data ) -= val; return *((double *) data);} return UD_DOUBLE;}
			double							operator *= ( double val ) { if( data ) { *(( double *) data ) *= val; return *((double *) data);} return UD_DOUBLE;}
			double							operator /= ( double val ) { if( data ) { *(( double *) data ) /= val; return *((double *) data);} return UD_DOUBLE;}

			int								size() override { return ( data ) ? siz : 0; }
			double							value(){ return ( data ) ? *( double *) data : 0.0; }
			double							doubleValue() { return ( data ) ? *( double *) data : 0.0; }
			void							set( const double &d ){ *((double*) data) = d; }
			float							floatValue(){ if( data ) return ( float ) *( ( double *) data ); return 0.0; }
			std::string						*toNetString();                      // convert to net string format
			std::string						*toJsonString();                     // convert to json string format
			const char						*c_str();
			void							dump( FILE *fp = stderr ) override ;
			void							cdump( FILE *fp = stderr ) override ;
};


class CONull : public CppON
{
public:
											CONull( CONull &nt ) : CppON( NULL_CPPON_OBJ_TYPE ){}
											// cppcheck-suppress noExplicitConstructor
											CONull( CONull *nt ) : CppON( NULL_CPPON_OBJ_TYPE ){}
											CONull( ) : CppON( NULL_CPPON_OBJ_TYPE ){}

			int								size() override { return 0; }
			void							*value(){ return NULL; }
			std::string						*toNetString() { return new std::string( "0:~" ); }
			std::string						*toJsonString();
	const	char							*c_str(){ return "NULL"; }
			void							dump( FILE *fp = stderr ) override ;
			void							cdump( FILE *fp = stderr ) override ;
			CONull							*operator =( CONull *val ){ return this; }
			CONull							*operator = ( CONull &val ){ return this; }
			bool							operator == ( CONull &newObj ) { return true; }
			bool							operator == ( CONull *newObj ) { return true; }
			bool							operator != ( CONull &newObj ) { return false; }
			bool							operator != ( CONull *newObj ) { return false; }
};

class COBoolean : public CppON
{
public:
											COBoolean( COBoolean &bt ): CppON( BOOLEAN_CPPON_OBJ_TYPE ){ if( !data ) { data = new( bool ); } *( ( bool *) data) = bt.value(); siz = sizeof( bool ); }
											COBoolean( COBoolean *bt = NULL ): CppON( BOOLEAN_CPPON_OBJ_TYPE ){ if( !data ) { data = new( bool ); } *( ( bool *) data) = bt->value(); siz = sizeof( bool ); }
											COBoolean( bool v = false ) : CppON( BOOLEAN_CPPON_OBJ_TYPE ){ if( !data ) { data = new( bool ); } *( ( bool *) data) = v; siz = sizeof( bool ); }
			int								size() override { return ( data ) ? sizeof( bool ) : 0; }
			bool							value(){ return ( ( data ) ? ( ( *( ( bool *) data ) ) ? true : false ) : false ); }
			std::string						*toNetString();                      // convert to net string format
			std::string						*toJsonString();                      // convert to json string format
			const char						*c_str();
			void							dump( FILE *fp = stderr ) override ;
			void							cdump( FILE *fp = stderr ) override ;
			bool							operator == ( COBoolean &newObj ) { return ( *((bool *)newObj.data) == *( (bool *) data ) ); }
											// cppcheck-suppress constParameter
			bool							operator == ( COBoolean *newObj ) { return ( *this == *newObj  ); }
											// cppcheck-suppress constParameter
			bool							operator != ( COBoolean &newObj ) { return ( ! ( *this == newObj ) );}
											// cppcheck-suppress constParameter
			bool							operator != ( COBoolean *newObj ) { return ( ! ( *this == *newObj ) );}
			bool							operator = ( bool val ) { *( ( bool *) data) = val; return val; }
			COBoolean						*operator = ( COBoolean &val) { *( ( bool *) data) = val.value(); return this; }
			COBoolean						*operator = ( COBoolean *val) { *( ( bool *) data) = val->value(); return this; }
};


class COString : public CppON
{
public:
											COString( COString &st );
											COString( COString *st = NULL );
											COString( const char *st = "", bool base64 = false );
											COString( std::string st, bool base64 );
											COString( std::string st = std::string("") );
											COString( uint64_t val, bool hex = true );
											COString( uint32_t val, bool hex = true );
	static	char							*base64Decode( const char *tmp, unsigned int sz, unsigned int &len, char *out = NULL );
			COString						*append( std::string &val ) { if( data ) ( ( std::string *) data)->append( val ); else data = new std::string( val );  return this; }
			COString						*append( const char *val ) { if( data ) ( ( std::string *) data)->append( val ); else data = new std::string( val );  return this; }
			COString						*operator += ( const char *val ) { if( data ) ((std::string *) data)->append( val ); else data = new std::string( val );  return this; }
			COString						*operator += ( std::string &val ) { if( data ) ((std::string *) data)->append( val ); else data = new std::string( val ); return this; }
			COString						*operator = ( const char *val ) { if( data ) delete((std::string*) data ); data = new std::string( val ); return this; }
			COString						*operator = ( std::string &val) { if( data ) delete((std::string*) data ); data = new std::string( val.c_str() ); return this; }
			COString						*operator = ( COString &val) { if( data ) delete((std::string*) data ); data = new std::string( val.c_str() ); return this; }
											// cppcheck-suppress constParameter
			COString						*operator = ( COString *val) { return( *this = *val ); }
			COString						*operator = ( uint64_t val );
			COString						*operator = ( uint32_t val );
			COString						*operator = ( int val );
			bool							operator == ( COString &newObj ) { return ( ! ( ( std::string *) data )->compare( ( ( std::string *)newObj.data)->c_str( ) ) ); }
											// cppcheck-suppress constParameter
			bool							operator == ( COString *newObj ) { return ( *this == *newObj ); }
			bool							operator != ( COString &newObj ) { return (   ( ( std::string *) data )->compare( ( ( std::string *)newObj.data)->c_str( ) ) ); }
											// cppcheck-suppress constParameter
			bool							operator != ( COString *newObj ) { return ( *this != *newObj ); }

			int								size() override { return ( ( data != NULL ) ? ( ( std::string * ) data )->length() : 0 ); }

			const char						*c_str(){ return ( (std::string *) data )->c_str(); }
			std::string						*value(){ return ( data != NULL )? ( std::string * ) data : NULL; }
			std::string						*toString();
			std::string						*toNetString();                      										// convert to net string format
			std::string						*toJsonString();                    										// convert to JSON string format
static		std::string						*toBase64JsonString( const char *cPtr, unsigned int len );					// convert to base64 encoded JSON string
			std::string						*toBase64JsonString(){ return toBase64JsonString( ( ( std::string *) data )->c_str(), ( ( std::string *) data )->length() ); }
			void							dump( FILE *fp = stderr ) override ;
			void							cdump( FILE *fp = stderr ) override ;
};


class COMap : public CppON
{
public:
											COMap( COMap &mt );
											// cppcheck-suppress noExplicitConstructor
											COMap( COMap *mt );
											// cppcheck-suppress noExplicitConstructor
											COMap( const char *str );
											COMap( const char *path, const char *file );
											COMap( ) : CppON(  MAP_CPPON_OBJ_TYPE ) { data = new std::map<std::string, CppON*>(); }
											// cppcheck-suppress noExplicitConstructor
											COMap( std::map < std::string, CppON *> &m ) : CppON( MAP_CPPON_OBJ_TYPE ){ data = new std::map<std::string, CppON *>( m ); }
			int								size() override { return ( data ) ? ((std::map< std::string, CppON*> *) data)->size() : 0; }

std::map<std::string,CppON*>::iterator		begin() { return ((std::map< std::string, CppON*> *) data)->begin(); }
std::map<std::string,CppON*>::iterator		end() { return ((std::map< std::string, CppON*> *) data)->end(); }
			COMap							*operator = ( const char *str);
			COMap							*operator = ( COMap &val );
											// cppcheck-suppress constParameter
			COMap							*operator = ( COMap *val ){ return( *this = *val ); }
			bool							operator == ( COMap &val );
											// cppcheck-suppress constParameter
			bool							operator == ( COMap *val ){ return ( *this == *val  ); }
											// cppcheck-suppress constParameter
			bool							operator != ( COMap &val ){ return ( ! (*this == val ) );}
											// cppcheck-suppress constParameter
			bool							operator != ( COMap *val ){ return ( ! (*this == *val ) );}
			std::vector<std::string>		*getKeys() { return &order; }
			std::vector<CppON *>			*getValues();
			std::map< std::string, CppON*>	*value() { return ( data ) ? ( std::map < std::string, CppON *> *) data : NULL; }
			std::string						*toNetString();
			CppON							*extract( const char *name );
			int								append( std::string key, CppON *n );
			int								append( std::string key, std::string value ){ return append( key, new COString( value ) ); }
			int								append( std::string key, const char *value){ return append( key, new COString( value ) ); }
			int								append( std::string key, double value ){ return append( key, new CODouble( value ) ); }
			int								append( std::string key, int64_t value ){ return append( key, new COInteger( value ) );}
			int								append( std::string key, int value ){ return append( key, new COInteger( value ) ); }
			int								append( std::string key, bool value ){ return append( key, new COBoolean( value ) ); }
			int								append( std::string key ){ return append( key, new CONull() ); }
			void							removeVal( std::string val );
			void							replaceObj( std::string s, CppON *obj );
			void							clear();
			CppON							*findEqual( const char *name, CppON &search );
			CppON							*findElement( const char *str );
			CppON							*findNoSplit( const char *str );
			CppON							*findElement( const std::string &str ) { return findElement( str.c_str() ); }
			CppON							*findElement( const std::string *str ) { return findElement( str->c_str() ); }
			CppON							*findElement( std::string *s ) { return findElement( s->c_str() ); }
			CppON							*findCaseElement( const char *str );
			CppON							*findCaseElement( const std::string &str ) { return findCaseElement( str.c_str() ); }
			CppON							*findCaseElement( const std::string *str ) { return findCaseElement( str->c_str() ); }
			std::string						*toJsonString( std::string &indent );
			std::string						*toJsonString(){ std::string indent(""); return toJsonString( indent ); }
			std::string						*toCompactJsonString();
			const char						*c_str( std::string &indent );
			const char						*c_str(){ std::string indent(""); return c_str( indent ); }
			int								toFile( const char *path );
			void							dump( std::string &indent, FILE *fp = stderr );
			void							dump( FILE *fp = stderr )  override { std::string indent(""); dump( indent, fp ); fprintf( fp, "\n" );}
			void							cdump( FILE *fp = stderr ) override ;
			COMap							*diff( COMap &newObj, const char *name = NULL);
			void							upDate( COMap *map, const char *name );
			void							merge( COMap *map, const char *name );
private:
			void							parseData( const char *str );
};

class COArray : public CppON
{
public:
											COArray( COArray &at );
											// cppcheck-suppress noExplicitConstructor
											COArray( COArray *at );
											// cppcheck-suppress noExplicitConstructor
											COArray( const char *str );
											// cppcheck-suppress noExplicitConstructor
											COArray( const char *path, const char *file );
											// cppcheck-suppress noExplicitConstructor
											COArray( ) : CppON( ARRAY_CPPON_OBJ_TYPE ) { data = new std::vector<CppON *>(); }
											// cppcheck-suppress noExplicitConstructor
											COArray( std::vector<CppON *> &v ) : CppON( ARRAY_CPPON_OBJ_TYPE ){ data = new std::vector<CppON *>( v ); }
			int								size() override { return ( data ) ? (( std::vector<CppON *> *) data)->size() : 0; }
			std::vector< CppON *>			*value() { return ( data ) ? ( std::vector< CppON *> *) data : NULL; }
			std::vector< CppON* >::iterator	begin() { return ((std::vector< CppON*> *) data)->begin(); }
			std::vector< CppON* >::iterator	end() { return ((std::vector< CppON*> *) data)->end(); }

			std::string						*toNetString();
			bool							replace( size_t i, CppON *n){ std::vector<CppON *> *v = (std::vector< CppON *> *) data; if( v->size() > i ) { delete( (*v)[ i ] ); (*v)[ i ] = n; return true;} return false; }
			bool							operator == ( COArray &val );
											// cppcheck-suppress constParameter
			bool							operator == ( COArray *val ){ return( *this == *val ); }
											// cppcheck-suppress constParameter
			bool							operator != ( COArray &val ){ return( ! (*this == val ) ); }
											// cppcheck-suppress constParameter
			bool							operator != ( COArray *val ){ return( ! (*this == *val ) ); }
			COArray							*operator = ( COArray &val );
											// cppcheck-suppress constParameter
			COArray							*operator = ( COArray *val ){ return( *this = *val ); }
			CppON							*remove( size_t idx );
			void							append( CppON *n ) { ( (std::vector < CppON *> *) data)->push_back( n ); }
			void							append( std::string value ){ append( new COString( value ) ); }
			void							append( double value ){ append( new CODouble( value ) ); }
			void							append( int64_t value ){ append( new COInteger( value ) ); }
			void							append( int value ){ append( new COInteger( value ) ); }
			void							append( bool value ) { append( new COBoolean( value ) ); }
			void							push_back( CppON *n ){ ( (std::vector < CppON *> *) data)->push_back( n ); }
			CppON							*pop( ){ return remove( size() - 1 ); }
			CppON							*pop_front(){ return remove( 0 ); }
			void							push( CppON *n) { append( n ); }
			void							clear();
			CppON							*at( unsigned int i )
											{
												if( ! data || ((std::vector < CppON *> *) data)->size() <= i )
												{
													return NULL;
												}
												return ((std::vector < CppON *> *) data)->at( i );
											}
			std::string					*toJsonString( std::string &indent );
			std::string					*toJsonString(){ std::string indent(""); return toJsonString( indent ); }
			std::string					*toCompactJsonString();
	const	char						*c_str( std::string &indent );
	const	char						*c_str(){ std::string indent(""); return c_str( indent ); }
			void						dump( std::string &indent, FILE *fp = stderr );
			void						dump( FILE *fp = stderr ) override { std::string indent; dump( indent, fp ); }
			void						cdump( FILE *fp = stderr ) override ;
			COArray						*diff( COArray &newObj, const char *name = NULL);
private:
			void							parseData( const char *str );
};

#endif /* CPPON_HPP_ */
