/*
 *
 *   File Name: CppON.cpp
 *  Created on: 10 Jan 2010 - 26 May 2024
 *      Author: Jeff Hertig
 * Description: JSON made easy in C++.  This is a library used to implement the concept of json, xml and tnetstrings in C++ as a hierarchical class structure
 * 				and much, much more. It is a very powerful and useful library in that it converts different string representations of data structures to and
 * 				from the class library.  This enables easy creation and manipulation of data in c++ and easy conversion back in forth to string formats for
 * 				storage and transmission over a serial link. Class structure can be created by loading character data directly from files, C-strings,
 * 				std::strings or they can be constructed completely through C code using the new operator.
 *
 * 				The class hierarchy is based completely on JSON.  There are Two container classes the "Map" (COMap) and the "Array" (COArray).  And, five
 * 				object low level object classes boolean (COBoolean), real numbers (CODouble), Integer numbers (COInteger), Strings (COString), and the
 * 				Null or undefined type( CONull).  All are derived from the CppON base class and can be cast to it.
 *
 *              It should be noted, that once created and added into the hierarchy, the root node "owns" all the branches of the hierarchy and will
 *              destroy them when destroyed.  This means you should be careful to not add a "stack created" item to a hierarchy as it will be destroyed
 *              when the code loses scope and also when the hierarchy is destroyed resulting in a crash.
 *
 *              The root of most "trees" is usually one of the two "container" classes: the Map object or an Array.  Usually an Map.
 *              A Map represents a hashmap and is actual implemented as an extension of a std::map< std::string, CppON *> container.  It is
 *              represented as a map in JSON and TNetStrings as well as an object in JavaScript.  In fact the method value() returns a pointer
 *              to the internal map.  Care should be used not to modify the contents returned from this function.
 *              An Array represents a dynamic list of objects and is implement as extension of a std::vector< CppON *> container internally. Like
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
 *              COArray      -  Container class: Implemented as a stl vector it is basically an array container which
 *                              can hold a list of objects including other COArrays.
 *              COMap        -  Container class: Implements a hash_map type container of key:value pares where the
 *                              value can be any of the possible objects.  Keys must be strings but
 *                              values can be any of these objects including other COMaps.
 *
 *              COBoolean    -  This represents a boolean value of true or false.
 *              CODouble     -  This represents a floating point number
 *              COInteger    -  This represents an integer number but can be a char, short, int, long
 *                              or long long.
 *              COString     -  This represents a character string it acts much like the std::string object
 *              CONull       -  represents a undefined value.
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
 *
 *    Depends:  This Library depends on STL:map, STL:string, STL: vector and the "jansson" library and xml2 if XML support is desired.
 */

#include "CppON.hpp"

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

#define DIRECTORY_BIT 0x4000

using namespace std;

CppON *CppON::factory( CppON &jt )
{
    CppON *rtn = NULL;

    CppONType typ = jt.type();                   // This is used to indicate the object type
    switch( typ )
    {
        case INTEGER_CPPON_OBJ_TYPE:
            rtn = new COInteger( (COInteger &) jt );
            break;

        case DOUBLE_CPPON_OBJ_TYPE:
            rtn = new CODouble( (CODouble &) jt );
            break;

        case STRING_CPPON_OBJ_TYPE:
            rtn = new COString( (COString &) jt );
            break;

        case NULL_CPPON_OBJ_TYPE:
            rtn = new CONull;
            break;

        case BOOLEAN_CPPON_OBJ_TYPE:
            rtn = new COBoolean( (COBoolean &) jt );
            break;

        case MAP_CPPON_OBJ_TYPE:
            rtn = new COMap( (COMap &) jt );
            break;

        case ARRAY_CPPON_OBJ_TYPE:
            rtn = new COArray( (COArray &) jt );
            break;

        default:
            break;
    }
    return rtn;
};

CppON::CppON( CppONType type )
{
    typ = type;
    siz = 0;
    data = NULL;
    precision = -1;
}

// cppcheck-suppress constParameter
CppON::CppON( CppON *jt )
{
    siz = jt->siz;
    data = NULL;
    precision = jt->precision;
    switch ( typ = jt->typ )
    {
    	case INTEGER_CPPON_OBJ_TYPE:
    		*this = *(new COInteger( (COInteger*) jt ) );
    		break;
    	case DOUBLE_CPPON_OBJ_TYPE:
    		*this = *(new CODouble( (CODouble*) jt ));
    		break;
       	case STRING_CPPON_OBJ_TYPE:
    		*this = *(new COString( (COString*) jt ));
    		break;
    	case BOOLEAN_CPPON_OBJ_TYPE:
    		*this = *(new COBoolean( (COBoolean*) jt ));
    		break;
    	case MAP_CPPON_OBJ_TYPE:
    		*this = *(new COMap( (COMap*)jt ));
    		break;
    	case ARRAY_CPPON_OBJ_TYPE:
    		*this = *(new COArray( (COArray*)jt ) );
    		break;
    	default:
    		break;
    }
}
// cppcheck-suppress constParameter
CppON::CppON( CppON &jt )
{
    siz = jt.siz;
    data = NULL;
    precision = jt.precision;
    CppON *ptr = &jt;

    switch ( typ = jt.typ )
    {
    	case INTEGER_CPPON_OBJ_TYPE:
    		*this = *(new COInteger( (COInteger *) ptr ) );
    		break;
    	case DOUBLE_CPPON_OBJ_TYPE:
    		*this = *(new CODouble( (CODouble *) ptr ) );
    		break;
       	case STRING_CPPON_OBJ_TYPE:
    		*this = *(new COString( (COString *) ptr ) );
    		break;
    	case BOOLEAN_CPPON_OBJ_TYPE:
    		*this = *(new COBoolean( (COBoolean *) ptr ) );
    		break;
    	case MAP_CPPON_OBJ_TYPE:
    		*this = *(new COMap( (COMap *) ptr ) );
    		break;
    	case ARRAY_CPPON_OBJ_TYPE:
    		*this = *(new COArray( (COArray *) ptr ) );
    		break;
    	default:
    		break;
    }
}

void CppON::deleteData()
{
    if( data )
    {
        switch( typ )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                switch( siz )
                {
                    case 1:
                        delete( (char *) data );
                        break;

                    case 2:
                        delete( (short *) data );
                        break;

                    case sizeof(int):
                        delete( (int *) data );
                        break;

                    case sizeof( long long ):
                        delete( ( long long *) data );
                        break;

                    default:
                        break;
                }
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                delete( (double *) data );
                break;
            case BOOLEAN_CPPON_OBJ_TYPE:
                delete( (bool *) data );
                break;
            case NULL_CPPON_OBJ_TYPE:
                break;
            case STRING_CPPON_OBJ_TYPE:
                delete (std::string *) data;
                break;
            case MAP_CPPON_OBJ_TYPE:
                {
                    map <string, CppON*> *m = ( map <string, CppON *> * ) data;
                    map<string, CppON *>::iterator it;
                    // cppcheck-suppress postfixOperator
                    for( it = m->begin(); m->end() != it; it++ )
                    {
                        delete( it->second);
                    }
                    m->clear();
                    delete ( m );
                }
                break;
            case ARRAY_CPPON_OBJ_TYPE:
                {
                    vector <CppON *> *v = ( vector<CppON *> * ) data;
                    for(unsigned int i = 0; v->size() > i; i++ )
                    {
                        delete( v->at( i ) );
                    }
                    delete ( v );
                }
                break;
            default:
                break;
        }
        data = NULL;
    }
    order.clear();
}

CppON::~CppON()
{
    deleteData();
}

bool CppON::operator == ( CppON &obj )
{
    bool rtn = false;

    if( typ != obj.typ )
        return rtn;

    switch (typ)
    {
        case INTEGER_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((COInteger *)this) == *((COInteger *) &obj);
            break;

        case DOUBLE_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((CODouble *)this) == *((CODouble *) &obj);
            break;

        case STRING_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((COString *)this) == *((COString *) &obj);
            break;

        case NULL_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((CONull *)this) == *((CONull *) &obj);
            break;

        case BOOLEAN_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((COBoolean *)this) == *((COBoolean *) &obj);
            break;

        case MAP_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((COMap *)this )  == *((COMap *) &obj );
            break;

        case ARRAY_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = *((COArray *)this )  == *((COArray *) &obj );
            break;

        default:
            break;
    }
    return rtn;
}

CppON *CppON::guessDataType( const char *str )
{
    CppON      *rtn = NULL;
    CppONType   _eType = NULL_CPPON_OBJ_TYPE;

    if( str && *str )
    {
        if( !strcasecmp( str, "true" ) || !strcasecmp( str, "false" ) )    // If it is true or false its boolean
        {
            _eType = BOOLEAN_CPPON_OBJ_TYPE;
        } else {
            char  _ch;

            _eType = INTEGER_CPPON_OBJ_TYPE;                                 // Assume integer
            for( int i = 0; (_ch = str[ i ]); i++ )                         // Check each character
            {
                if( '.' == _ch )                                            // If we have a decimal point it may be double
                {
                    if( DOUBLE_CPPON_OBJ_TYPE == _eType )                    // Two decimal points means it can't be double
                    {
                        _eType = STRING_CPPON_OBJ_TYPE;                      // String is default
                        break;
                    }
                    _eType = DOUBLE_CPPON_OBJ_TYPE;
                } else if( '0' > _ch || '9' < _ch ) {                       // Any character that isn't a number or decimal
                    _eType = STRING_CPPON_OBJ_TYPE;                          // means string;
                    break;
                }
            }
        }
    }

    switch( _eType )
    {
        case INTEGER_CPPON_OBJ_TYPE:
            rtn = new COInteger( strtoll( str, NULL, 10 ) );
            break;

        case DOUBLE_CPPON_OBJ_TYPE:
            rtn = new CODouble( strtod( str, NULL ) ) ;
            break;

        case BOOLEAN_CPPON_OBJ_TYPE:
            rtn = new COBoolean( 0 == strcasecmp( str, "true" ) );
            break;

        case STRING_CPPON_OBJ_TYPE:
            rtn = new COString( str );
            break;

        case MAP_CPPON_OBJ_TYPE:
            rtn = new COMap();
            break;

        case ARRAY_CPPON_OBJ_TYPE:
            rtn = new COArray();
            break;

        default:
            rtn = new CONull();
            break;
    }

    return rtn;
}

string *CppON::toNetString( const char *str, char styp )
{
    int len = strlen( str );
    char buf[ len + 16 ];
    snprintf( buf, len+16, "%d:%s%c", len, str, styp );

    return new string( buf );
};

const char *CppON::c_str()
{
    string indent = "";
    str = "";

    if( data )
    {
        switch( typ )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (COInteger *) this )->c_str();
                break;

            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (CODouble *) this )->c_str();
                break;

            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (COString *) this )->c_str();
                break;

            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (CONull *) this )->c_str();
                break;

            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (COBoolean *) this )->c_str();
                break;

            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (COMap *) this )->c_str( indent);
                break;

            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                str = ( (COArray *) this )->c_str( indent);
                break;

            default:
                str = "Unknown";
                break;
        }
    }
    return str.c_str();
}

// cppcheck-suppress unusedFunction
std::string *CppON::toCompactJsonString()
{
    std::string *sptr = NULL;
    if( data )
    {
        switch( typ )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (COInteger *) this )->toJsonString();
                break;

            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (CODouble *) this )->toJsonString();
                break;

            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (COString *) this )->toJsonString();
                break;

            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (CONull *) this )->toJsonString();
                break;

            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (COBoolean *) this )->toJsonString();
                break;

            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (COMap *) this )->toCompactJsonString();
                break;

            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                sptr = ( (COArray *) this )->toCompactJsonString();
                break;
            default:
                break;
        }
    }

    return sptr;
}

// cppcheck-suppress unusedFunction
void CppON::dump( FILE *fp )
{
    string indent = "";

    if( data )
    {
        switch( typ )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COInteger *) this )->dump( fp );
                break;

            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (CODouble *) this )->dump( fp );
                break;

            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COString *) this )->dump( fp );
                break;

            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (CONull *) this )->dump( fp );
                break;

            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COBoolean *) this )->dump( fp );
                break;

            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COMap *) this )->dump( indent, fp );
                break;

            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COArray *) this )->dump( indent, fp );
                break;

            default:
                fprintf( fp, "Unknown\n");
                break;
        }
    }
    fprintf( fp, "\n" );
}

void CppON::cdump( FILE *fp )
{
    if( data )
    {
        switch( typ )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COInteger *) this )->cdump( fp );
                break;

            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (CODouble *) this )->cdump( fp );
                break;

            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COString *) this )->cdump( fp );
                break;

            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (CONull *) this )->cdump( fp );
                break;

            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COBoolean *) this )->cdump( fp );
                break;

            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COMap *) this )->cdump( fp );
                break;

            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                ( (COArray *) this )->cdump( fp );
                break;
            default:
                fprintf( fp, "Unknown");
                break;
        }
    }
}


CppON *CppON::parseJson( json_t *ob, string &tabs )
{
    CppON    *rtn = NULL;
    if( ob )
    {
        if( json_is_string( ob ) )
        {
            rtn = new COString( json_string_value( ob ) );

        } else if( json_is_number( ob ) ) {
            if( json_is_integer( ob) )
            {
                rtn = new COInteger( (long long) json_integer_value( ob ) );
            } else if( json_is_real( ob ) ) {
                rtn = new CODouble( json_real_value( ob ) );
            }

        } else if( json_is_boolean( ob ) ) {
            rtn = new COBoolean( json_is_true( ob ) );

        } else if( json_is_object( ob ) ) {
            const char *key;
            json_t   *value;
            COMap   *mp = new COMap();
            string  osave = tabs;
            tabs += '\t';

            json_object_foreach( ob, key, value )
            {
                mp->append( key, parseJson( value, tabs ) );
            }
            tabs = osave;
            rtn = mp;

        } else if( json_is_array( ob ) ) {
            COArray  *ap = new COArray();
            string asave = tabs;
            tabs += '\t';
            for( unsigned int i = 0; json_array_size( ob ) > i; i++ )
            {
                ap->append( parseJson( json_array_get( ob, i ), tabs ) );
            }
            tabs = asave;
            rtn = ap;

        } else if( json_is_null( ob ) ) {
            rtn = new CONull();
        }
    }
    return rtn;
}

CppON *CppON::parseJson( const char *str )
{
    CppON     *rtn = NULL;
    string tabs = "";

    if( str )
    {
        json_t      *root = NULL;
        json_error_t  error;
        char      *np;
        int       cnt = strtol( str, &np, 0 );      								// Check to see if it is a tnetstring
        if( np )
        {
            np++;
            if( cnt && ',' == np[ cnt ] )
            {
                char *tmp = strdup( np );                							// make a copy so it can be modified
                tmp[ cnt ] = '\0';                      							// remove the ',' at  the end of it
                root = json_loads( tmp, 0, &error );    							// load it as JSON
                free( tmp );
            } else if ( cnt ) {
                root = json_loads( np, 0, &error );      							// load it as JSON
            }
        }

    	while( ' ' == *str || '\t' == *str || '\n' == *str )						// Trim beginning of string
    	{
    		str++;
    	}
        if( '"' != str[ 0 ] )
        {
            if( !root )                                  							// I guess it wasn't TNetstring so load it as JSON
            {
                if( !( str && str[ 0 ] ) )
                {
                    fprintf( stderr, "%s[%d] Error: Attempt to parse zero length JSON string\n",__FILE__, __LINE__ );
                }
                root = json_loads( str, 0, &error );
            }
            if( ! root  )
            {
                fprintf( stderr, "%s[%d] Error: on line %d %s\n", __FILE__,__LINE__, error.line, error.text );
                fprintf( stderr, "%s\n",str );
                return NULL;
            }

            rtn = parseJson( root, tabs );
            json_decref( root );
        } else {
        	std::string s( "{" );
        	s.append( str );
        	s += "}";

            if( !root )                                  							// I guess it wasn't TNetstring so load it as JSON
            {
                root = json_loads( s.c_str(), 0, &error );
            }
            if( ! root  )
            {
                fprintf( stderr, "%s[%d] Error: on line %d %s\n", __FILE__,__LINE__, error.line, error.text );
                fprintf( stderr, "%s\n",str );
            } else {
            	COMap *mp = (COMap *) parseJson( root, tabs );
            	rtn = mp->extract( mp->begin()->first.c_str() );
            	delete( mp );
            	json_decref( root );
            }
        }
    } else {
        fprintf( stderr, "ERROR: Asked to parse NULL JSON string\n" );
    }
    return rtn;
}

// cppcheck-suppress unusedFunction
CppON *CppON::parseJsonFile( const char *path )
{
    int     sz = 1024;
    char    *buf,*bSave;
    FILE    *fp = fopen( path, "r" );
    int     rd = 0;
    int     c;
    CppON *rtn = NULL;

    if( !fp )
    {
        char estr[ 1024 ];
        snprintf( estr, 1023, "fopen Failed to open JSON FILE \"%s\"", path );
        perror( estr );
        return rtn;
    }

    if( !( buf = (char *) malloc( sz ) ) )
    {
        fclose( fp );
        perror( "malloc Memory allocation error" );
        return rtn;
    }

    while( EOF != ( c = fgetc( fp ) ) )
    {
        buf[ rd++ ] = ( char ) c;
        if( rd == sz )
        {
            if( !( buf = (char *) realloc( (void *) (bSave = buf ), sz += 512 ) ) )
            {
                snprintf( bSave, 1023, "realloc Failed to allocate %d bytes of memory", sz );
                perror( bSave );
                free( bSave );
                return rtn;
         }
        }
    }
    buf[ rd ] = '\0';
    fclose( fp );

    rtn = parseJson( buf );
    free(buf );

    return ( rtn );
}

CppON *CppON::readObj( FILE *fp )
{
	CppON 		*rtn = NULL;
	int			otype = -1;
	int			stype = -1;
	unsigned	rd = 0;
	int			c;
	unsigned	levels = 0;
	bool 		done = false;

	if( fp )
	{
	    unsigned	sz = 1024;
	    char    	*buf,*bSave;

	    if( !( buf = (char *) malloc( sz ) ) )
	    {
	        fclose( fp );
	        perror( "malloc Memory allocation error" );
	    } else {
	    	buf[ 0 ] = '\0';
	    	while( 0 > otype && EOF != ( c = fgetc( fp ) ) )								// Read characters until we get the first character of the json
	    	{
	    		if( '{' == c )																// Start of a Map?
	    		{
	    			levels = 1;
	    			otype = 0;
	    		} else if( '\"' == c ) {													// Start of an Object?
	    			levels = 1;
	    			otype = 2;
	    		} else if( '[' == c ) {														// Start of an Array?
	    			levels = 1;
	    			otype = 1;
	    		}
	    	}

	    	if( 0 <= otype )																// if we got a '"', '{' or a '[' then we may have something
	    	{
	    		buf[ rd++ ] = c;
	    	    buf[ rd ] = '\0';
	    	    while( !done && EOF != ( c = fgetc( fp ) ) )
	    	    {
	    	        buf[ rd++ ] = ( char ) c;

	    	        if( rd == sz )
	    	        {
	    	            if( !( buf = (char *) realloc( (void *) (bSave = buf ), sz += 512 ) ) )
	    	            {
	    	                perror( bSave );
	    	                free( bSave );
	    	                break;
	    	            }
	    	        }
		    	    buf[ rd ] = '\0';

	    	        switch ( otype )
	    	        {
	    	        	case 0:
	    	        		if( '}' == c )
	    	        		{
	    	        			if( !( --levels ) )
	    	        			{
	    	        				done = true;
	    	        				break;
	    	        			}
	    	        		} else if( '{' == c ) {
	    	        			levels++;
	    	        		}
	    	        		break;
	    	        	case 2:
	    	        		if( 1 == levels && '\"' == c )
	    	        		{
	    	        			levels = 2;
	    	        		} else if( 2 == levels && ':' == c ) {
	    	        			levels = 3;
	    	        		} else if( 3 == levels ) {
	    	        			if( 0 > stype )
	    	        			{
	    	        				if( '\"' == c )
	    	        				{
	    	        					stype = 0;
	    	        					levels = 4;
	    	        				} else if( '{' == c) {
	    	        					levels = 4;
	    	        					stype = 1;
	    	       					} else if( '[' == c ) {
	    	       						levels = 4;
	    	       						stype = 2;
	    	       					} else if ( ('0' <= c && c <= 9 ) || '-' == c || '+' == c ) {
	    	       						levels = 4;
	    	       						stype = 3;
	    	       					} else if( 't' == c || 'T' == c	) {								// true?
	    	       						stype = 4;
	    	       						levels = 4;
	    	       					} else if( 'f' == c || 'F' == c ) {								// false?
	    	       						stype = 5;
	    	       						levels = 4;
	    	       					}
	    	        			}
	    	        		} else if( 4 <= levels ) {
	    	        			if( 0 == stype  && '\"' == c )										// string
	    	        			{
 	    	        				done = true;
	    	        			} else if ( 1 == stype ) {											// map
	    	        				if( '{' == c )
									{
										levels++;
									} else if( '}' == c ) {
										if( 3 == --levels )
										{
											done = true;
										}
									}
	    	        			} else if ( 2 == stype ) {											// array
	    	        				if( '[' == c )
									{
										levels++;
									} else if( ']' == c ) {
										if( 3 == --levels )
										{
											done = true;
										}
									}
	    	        			} else if( 3 == stype ) {											// number
	    	        				if( ( '0' > c || '9' < c ) && '.' != c )
	    	        				{
	    	        					done = true;
	    	        				}
	    	        			} else if( 4 == stype ) {											// boolean true?
	    	        				char tc = '\0';
	    	        				switch( levels )
	    	        				{
	    	        					case 4:														// 'r'
	    	        						tc = 'r';
	    	        						break;
	    	        					case 5:														// 'u'
	    	        						tc = 'u';
	    	        						break;
	    	        					case 6:														// 'e'
	    	        						tc = 'e';
	    	        						break;
	    	        				}
	        						if( tc == tolower( c ) )
	        						{
	        							if( 7 == ++levels )
	        							{
	        								done = true;
	        								rtn = new COBoolean( true );
		        							free( buf );
		        							buf = NULL;
	        							}
	        						} else {
	        							done = true;
	        							free( buf );
	        							buf = NULL;
	        						}

	    	        			} else if( 5 == stype ) {											// boolean false?
	    	        				char tc = '\0';
	    	        				switch( levels )
	    	        				{
	    	        					case 4:														// 'a'
	    	        						tc = 'a';
	    	        						break;
	    	        					case 5:														// 'l'
	    	        						tc = 'l';
	    	        						break;
	    	        					case 6:														// 's'
	    	        						tc = 's';
	    	        						break;
	    	        					default:													// 'e'
	    	        						tc = 'e';
	    	        						break;
	    	        				}
	        						if( tc == tolower( c ) )
	        						{
	        							if( 8 == ++levels )
	        							{
	        								done = true;
	        								rtn = new COBoolean( false );
		        							free( buf );
		        							buf = NULL;
	        							}
	        						} else {
	        							done = true;
	        							free( buf );
	        							buf = NULL;
	        						}
	    	        			}

	    	        		}
        					break;
	    	        	case 1:
	    	        		if( ']' == c )
	    	        		{
	    	        			if( !( --levels ) )
	    	        			{
	    	        				done = true;
	    	        				break;
	    	        			}
	    	        		} else if( '[' == c ) {
	    	        			levels++;
	    	        		}
	    	        		break;
	    	        }
	    	    }
	    	}
	    }
		if( buf && buf[ 0 ] )
		{
		    rtn = parseJson( buf );
			free( buf );
		}
	}
	return rtn;
}



CppON  *CppON::diff( CppON &newObj, const char *name )
{

    if(  typ == newObj.typ )
    {
        switch ( typ )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                if( *( ( COInteger *) this ) != *(( COInteger *) &newObj) )
                {
                    return new COInteger( (( COInteger &) newObj) );
                }
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                if( *( ( CODouble *) this ) != *(( CODouble *) &newObj) )
                {
                    return new CODouble( (( CODouble &) newObj) );
                }
                break;
            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                if( *( ( COBoolean *) this ) != *(( COBoolean *) &newObj) )
                {
                    return new COBoolean( (( COBoolean &) newObj) );
                }
                break;
            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                return  ((COMap *) this)->diff( (COMap &)newObj, name );
                break;
            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                return  ((COArray *) this)->diff( (COArray &)newObj, name );
                break;
            case NULL_CPPON_OBJ_TYPE:
                fprintf(stderr, "CppON:diff - Null being ignored\n");
                break;
            default:
                break;
        }
    }
    return NULL;
}
#if HAS_XML

/*
 * Routine to trim white space off the beginning and end of the content of a text node
 */

static char *trimTag( xmlNodePtr con )
{
    char  *rtn = NULL;
    if( con )
    {
        if( !( xmlStrcmp( con->name, (const xmlChar *) "text" ) )  && con->content )        // Make sure it is a text node with content
        {
            char   *oPtr;
            char   *inPtr = (char *)con->content;
            char   *ePtr;
            char   ch;

            rtn  = new char[ strlen( (const char *) con->content ) + 1 ];                   // Create a string that is at least the same size
            oPtr = ePtr = rtn;                                                              //  as the original string

            while( ' ' == ( ch = *inPtr++ ) || '\n' == ch || '\t' == ch  );                 // Skip over Leading white space

            if( ch )                                                                        // If string had at least one non-white space character it is good
            {
                do                                                                          // Copy all characters until end of string
                {
                    *oPtr++ = ch;
                    if( ' ' != ch && '\t' != ch && '\n' != ch )                             // Remember place of last non-white space, + 1
                    {
                        ePtr = oPtr;
                    }
                } while( ( ch = *inPtr++ ) );                                               // Keep going until null is found

                *ePtr = '\0';                                                               // Put a null at place saved of space after last non white space
            } else {
                delete[] rtn;                                                               // empty string so delete it, and return NULL.
                rtn = NULL;
            }
        }
    }
    return rtn;
}

/*
 * Simple static routine to just check if the cursor has any non-text children.
 */

static bool hasChildren( xmlNodePtr cur )
{
    bool rtn = false;
    while( cur )
    {
        if( 1 == cur->type )
        {
            rtn = true;
        }
        cur = cur->next;
    }
    return rtn;
}

/*
 * Iterative routine to parse XML nodes.
 * A node may be a simple node of type string, integer, double, boolean or null
 * A map that contains children
 * An array that contains children
 * attributes are considered children and will have a '-' prepended to the name.
 * Nodes that have attributes or other children along with text will have the text added as a node named "#text"
 * This routing will call itself to resolve all of its children do do the simple node level
 */

static int parseNode( xmlNodePtr cur, CppON *parent, std:: string indent )
{
    xmlNodePtr           childCur;
    xmlNodePtr           cur2;
    xmlNodePtr          cur3;
    CppON              *root = NULL;
    CppON              *childObj = NULL;
    _xmlAttr             *attr = NULL;
    int                  rtn = 0;
    char                *value;
    static std::string  preStr = "";

    indent += "  ";
    if( !cur )
    {
        return 0;
    }

    if( 1 == cur->type )                                                                // Don't parse non-nodes
    {
        bool children = hasChildren( cur->xmlChildrenNode );
        if( ( attr = cur->properties ) || children )                                      // If it has properties it must be at least a map
        {
            if( parent->isMap() )
            {
                // cppcheck-suppress cstyleCast
                ((COMap *)parent )->append( (const char * ) cur->name, root = new COMap() );
            } else if( parent->isArray() ) {
                // cppcheck-suppress cstyleCast
                ((COArray *)parent )->append( root = new COMap() );
            }
            parent = root;

            /*
             * do attributes
             */

            for( ; attr; attr = attr->next )
            {
                std::string name = "-";                                                            // Attributes are children with the '-' prepended to their name
                name += (const char *) attr->name;

                childObj = CppON::guessDataType( (const char *) attr->children->content );  // decide data type from content

                if( parent->isMap() )                                                          // different routines to add node to array or map
                {
                    // cppcheck-suppress cstyleCast
                    ( ( COMap *) parent)->append( name, childObj );
                } else if( parent->isArray() ) {
                    // cppcheck-suppress cstyleCast
                    ( ( COArray *) parent)->append( childObj );
                }
            }

            if( ( childCur = cur->xmlChildrenNode ) )
            {
                for( cur2 = childCur; cur2; cur2 = cur2->next )                                // Process all children
                {
                    if( 1 == cur2->type )
                    {
                        bool         arrayMember = false;
                        std::string  tagName = (const char *)cur2->name;

                        for( cur3 = cur2->next; cur3; cur3 = cur3->next )                          // See if they are part of an array
                        {
                            if( !xmlStrcmp( (const xmlChar *)tagName.c_str(), cur3->name ) )
                            {
                                arrayMember = true;                                                    //    if the same name appears it must be an array
                            }
                        }
                        if( arrayMember )                                                          // Process all array children together
                        {
                            if( parent->isMap() )
                            {
                                // cppcheck-suppress cstyleCast
                                ( ( COMap *) parent)->append( tagName, childObj = new COArray() );
                            } else if( parent->isArray() ) {
                                // cppcheck-suppress cstyleCast
                                ( ( COArray *) parent)->append( childObj = new COArray() );
                            }
                            preStr = "\n  ";
                            preStr += indent;
                            parseNode( cur2, childObj, indent + "  " );

                            for( cur3 = cur2->next; cur3; )                                          // See if they are part of an array
                            {
                                xmlNodePtr cur4 = cur3;
                                cur3 = cur3->next;                                                    // Get next pointer in case we delete this one
                                if( !xmlStrcmp( (const xmlChar *)tagName.c_str(), cur4->name ) )
                                {
                                    parseNode( cur4, childObj, indent + "  " );
                                    xmlUnlinkNode( cur4 );
                                    xmlFreeNode( cur4 );
                                }
                            }
                        } else {
                            preStr = "";
                            parseNode( cur2, parent, indent );
                        }
                    } else if( ( value = trimTag( cur->xmlChildrenNode ) ) ) {                  // Node has attribute and content
                        // cppcheck-suppress cstyleCast
                        ( ( COMap *) parent)->append( "#text", childObj = new COString( value ) );
                        delete[] value;
                    }
                }
            }
        } else if( ( value = trimTag( cur->xmlChildrenNode ) ) ) {
            childObj = CppON::guessDataType( value );
            delete[] value;

            if( parent->isMap() )
            {
                // cppcheck-suppress cstyleCast
                ( ( COMap *) parent)->append( (const char *)cur->name, childObj  );
            } else if( parent->isArray() ) {
                // cppcheck-suppress cstyleCast
                ( ( COArray *) parent)->append( childObj );
            }
        }
    } else {
        if( (value = trimTag( cur ) ) )
        {
            childObj = CppON::guessDataType( value );
            delete[] value;

            if( parent->isMap() )
            {
                // cppcheck-suppress cstyleCast
                ( ( COMap *) parent)->append( indent, childObj );
            } else if( parent->isArray() ) {
                // cppcheck-suppress cstyleCast
                ( ( COArray *) parent)->append( childObj );
            }
        }
    }
    return rtn;
}

// cppcheck-suppress unusedFunction
CppON *CppON::parseXML( const char *str )
{
    CppON         *rtn = NULL;
    xmlDocPtr       doc;
    xmlNodePtr       cur;

    if( ! str )
    {
        fprintf( stderr, "Attempt to convert an empty string to a data Object\n");
    } else if( !( doc = ( '.' == *str || '/' == *str) ? xmlParseFile( str ) : xmlParseDoc( (const xmlChar *)str ) ) ) {
        fprintf( stderr, "Empty Document!\n" );
        xmlFreeDoc( doc );
    } else if( !(cur = xmlDocGetRootElement( doc ) ) ) {
        fprintf( stderr, "Failed to get root node!\n" );
        xmlFreeDoc( doc );
    } else {
        rtn = new COMap( );                                      // Create the new Map object to return
        parseNode( cur, rtn, std::string( "" ) );
        xmlFreeDoc( doc );
    }
    return rtn;
}
#endif

// cppcheck-suppress unusedFunction
CppON *CppON::parseTSV(const char *str )
{
    COArray   *rtn = NULL;
    if( ! str || ! str[ 0 ] )
    {
        fprintf( stderr, "Attempt to convert an empty string to a data Object\n");
    }  else {
        FILE *fd;
        if( ! ( fd = fopen( str, "r" ) ) )
        {
            fprintf( stderr, "Failed to open file %s: %d - %s\n",str, errno, strerror( errno ) );
        } else {
            int           ch = 0;
            std::string   s;
            COArray       *line= new COArray();

            rtn = new COArray();

            while( EOF != ( ch = fgetc( fd ) ) )
            {
                if( '\n' == ch )
                {
                    line->append( new COString( s ) );
                    s.clear();
                    rtn->append( line );
                    line = new COArray();
                } else if( '\t' == ch ) {
                    line->append( new COString( s ) );
                    s.clear();
                } else if( 31 <= ch && 127 > ch ) {
                    s += ch;
                }
            }

            delete( line );
            fclose( fd );
        }
    }
    return rtn;
}

// cppcheck-suppress unusedFunction
CppON *CppON::parseCSV(const char *str )
{
    COArray   *rtn = NULL;
    if( ! str || ! str[ 0 ] )
    {
        fprintf( stderr, "Attempt to convert an empty string to a data Object\n");
    }  else {
        FILE *fd;
        if( ! ( fd = fopen( str, "r" ) ) )
        {
            fprintf( stderr, "Failed to open file %s: %d - %s\n",str, errno, strerror( errno ) );
        } else {
            int           ch = 0;
            std::string   s;
            COArray       *line= new COArray();

            rtn = new COArray();

            while( EOF != ( ch = fgetc( fd ) ) )
            {
                if( '\n' == ch )
                {
                    line->append( new COString( s ) );
                    s.clear();
                    rtn->append( line );
                    line = new COArray();
                } else if( ',' == ch ) {
                    line->append( new COString( s ) );
                    s.clear();
                } else if( 31 <= ch && 127 > ch ) {
                    s += ch;
                }
            }

            delete( line );
            fclose( fd );
        }
    }
    return rtn;
}

/*
 * This routine parses a "TNetString" into a CppON object
 */
CppON *CppON::parse( const char *str, char **rstr )
{
    CppON    *rtn = NULL;

    if( str && *str )
    {
        COMap      *mp;
        COArray      *ar;
        char       *ptr, *ptr1;
        int     len = strtol( str, &ptr, 10 );

        ptr++;    // Point at next character

        if( rstr )
        {
            *rstr = &ptr[ len + 1 ];
        }
        switch ( ptr[ len ] )
        {
            case ',':
                {
                    char buf[ len + 1 ];
                    strncpy( buf, ptr, len );
                    buf[ len ]='\0';
                    rtn = new COString( string( buf ) );
                }
                break;

            case '#':
                {
                    long long ia = strtoll( ptr, NULL, 10);
                    rtn = new COInteger( ia );
                }
                break;

            case '^':
                {
                    double id = strtod( ptr, NULL );
                    rtn = new CODouble( id );
                }
                break;

            case '!':
                {
                    char buf[ len + 1 ];
                    strncpy( buf, ptr, len );
                    buf[ len ]= '\0';
                    rtn = new COBoolean( ( 0 == strcasecmp( "true", buf ) ) || ( 0 == strcasecmp( "t", buf ) ) );
                }
                break;

            case '~':
                rtn = new CONull();
                break;

            case '}':
                {
                    char buf[ len + 1 ];
                    strncpy( buf, ptr, len );
                    buf[ len ]='\0';
                    ptr = buf;
                    rtn = mp = new COMap();

                    while( CppON *mykey = CppON::parse( ptr, &ptr1 ) )
                    {
                        if( STRING_CPPON_OBJ_TYPE != mykey->type() )
                        {
                        	if( mykey )				// Should never happen but this will satisfy the stupid analyzer.
                        	{
                        		delete mykey;
                        	}
                        	delete mp;
                            return NULL;
                        }
                        if( CppON *myval = CppON::parse( ptr1, &ptr ) )
                        {
                            // cppcheck-suppress cstyleCast
                            mp->append( ((COString *)mykey)->c_str(), myval );
                            delete( mykey );
                        }
                    }
                }
                break;

            case ']':
                {
                    char buf[ len + 1];
                    strncpy( buf, ptr, len );
                    buf[ len ]='\0';
                    ptr = buf;

                    rtn = ar = new COArray();

                    while( CppON *myval = CppON::parse( ptr, &ptr1 ) )
                    {
                        ar->append( myval );
                        ptr = ptr1;
                    }
                }
                break;

            default:
                fprintf( stderr, "\t\tCppON:parse:unknown type %c\n", ptr[ len ] );
                //fprintf( stderr, "\t\t'%s'\n", str );
                break;

        }
    }
    return rtn;
}

/*
 * Little comparison function that returns 0 if two byte arrays are equal, 1 otherwise
 */
static int argCmp( const unsigned char *arg, int argSize, const unsigned char *val, int valSize )
{
    if( argSize != valSize )
        return 1;

    while( argSize-- )
        if( arg[ argSize ] != val[ argSize ] )
            return 1;
    return 0;
}

/*
 * Search a TNetString for a char string tag equal to an argument.
 *     arg - The null terminated character string you are looking for. (Yes it must be an ASCII C string)
 *     str - Pointer to the size part of the TNetString.  It must be terminated with a ':' per the spec
 *     next - Optional pointer used to return a pointer to the byte count of the next object in the string
 *     cnt - An optional pointer returning the size of the string.
 */

unsigned char *CppON::findTNetStringArg( const char *arg, int argSize, const char *str, const char **next, int *cnt )
{
    unsigned char   *rtn = NULL;
    unsigned char  *_cPtr;

    if( str && *str && arg && *arg )
    {
        if( '}' == *str || ']' == *str )
            str++;
        int _dSz = strtol( str, (char **)&_cPtr, 10 );

        if( _cPtr && ':' == *_cPtr++  )
        {
            unsigned char   _uc;

            if( '}' == ( _uc =  _cPtr[ _dSz ] ) )                        // object is hash map so check every one
            {
                char   *_cTmp = (char *)_cPtr;
                int    _dTmp;
                for( int sz = _dSz; 0 < sz; )
                {
                    if( ( rtn = CppON::findTNetStringArg( arg, argSize,  _cTmp, (const char **)&_cTmp, &_dTmp ) ) )
                    {
                        _dSz = _dTmp;
                        break;
                    }
                    if( 0 < (sz = _dSz - ( ( _cTmp - (char *)_cPtr ) +_dTmp + 1) ) && ',' == _cTmp[ -1 ] )
                    {
                        char  *_cTmp2;
                        _dTmp = strtol( _cTmp, &_cTmp2, 10 );

                        if( ',' == _cTmp2[ _dTmp + 1 ] )
                        {
                            _cTmp = _cTmp2 + _dTmp + 2;
                        }
                    }
                }
            } else if( ']' == _uc ) {                                // object is an array so check them
                char   *_cTmp = (char *)_cPtr;
                int    _dTmp;
                for( int sz = _dSz; 0 < sz; )
                {
                    if( ( rtn = CppON::findTNetStringArg( arg, argSize,  _cTmp, (const char **)&_cTmp, &_dTmp ) ) )
                    {
                        _dSz = _dTmp;
                        break;
                    }
                    sz = _dSz - ( ( _cTmp - (char *)_cPtr ) +_dTmp + 1 );
                }
            } else if( ',' == _uc && !argCmp( (unsigned char *) arg, argSize, _cPtr, _dSz ) ) {    // String, right length, equal
                                                                        //     Found tag now return next
                _cPtr += _dSz + 1;                                      // Point to size of next tag
                if( *_cPtr )                                            // If not EOS
                {
                    _dSz = strtol( (char *)_cPtr, (char **)&_cPtr, 10 );              // Get the size
                    if( ':' == *_cPtr++ )                              // Next char of a valid tag is ":"
                    {
                        rtn = _cPtr;
                    }
                }
            }
        }
        if( cnt )
        {
            *cnt = _dSz;
        }
        if( next && _cPtr )
        {
            *next = (char * )_cPtr + _dSz + 1;
        }
    } else {
        fprintf( stderr, "Error\n");
        if( arg )
        {
            fprintf( stderr,"arg = %s\n", arg );
        } else{
            fprintf( stderr, "arg == NULL\n" );
        }
        if( str )
        {
            fprintf( stderr, "str = %s\n", str );
        } else {
            fprintf( stderr, "str == NULL\n" );
        }
        if( cnt )
        {
            *cnt = 0;
        }
        if( next )
        {
            *next = NULL;
        }
    }
    return rtn;
}

long long CppON::toLongInt(void)
{
    long long rtn = 0;
    switch (typ)
    {
        case INTEGER_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = ((COInteger *)this)->longValue();
            break;

        case DOUBLE_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = (long long) ((CODouble *)this)->doubleValue();
            break;

        case STRING_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = strtol( ( ( COString *)this)->c_str(), NULL, 0 );
            break;

        case BOOLEAN_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = ( ((COBoolean *) this)->value() ) ? 1LL : 0LL;
            break;

        default:
            break;
    }

    return rtn;
}

int  CppON::toInt(void)
{
    int rtn = 0;
    switch (typ)
    {
        case INTEGER_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = ((COInteger *)this)->intValue();
            break;

        case DOUBLE_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = (int) ((CODouble *)this)->doubleValue();
            break;

        case STRING_CPPON_OBJ_TYPE:
         // cppcheck-suppress cstyleCast
            rtn = strtol( ( ( COString *)this)->c_str(), NULL, 0 );
            break;

        case BOOLEAN_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = ( ((COBoolean *) this)->value() ) ? 1 : 0;
            break;

        default:
            break;
    }
    return rtn;
}
bool CppON::toBoolean( void )
{
	bool	rtn = false;
	switch( typ )
	{
		case BOOLEAN_CPPON_OBJ_TYPE:
			rtn = ((COBoolean*) this)->value();
			break;
		case DOUBLE_CPPON_OBJ_TYPE:
			rtn = ( 0.0 != ((CODouble *) this)->doubleValue() );
			break;
		case INTEGER_CPPON_OBJ_TYPE:
			rtn = ( 0 != ((COInteger *) this)->intValue() );
			break;
        case STRING_CPPON_OBJ_TYPE:
        	rtn = ( 0 == strcasecmp( ( (COString *) this )->c_str(), "True" ) );
            break;
        default:
        	break;
	}
	return rtn;
}
double CppON::toDouble( void )
{
    double rtn = UD_DOUBLE;
    switch (typ)
    {
        case DOUBLE_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = ((CODouble *)this)->doubleValue();
            break;
        case INTEGER_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = (double)((COInteger *)this)->intValue();
            break;
        case STRING_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = strtod( ( ( COString *)this)->c_str(), NULL );
            break;
        case BOOLEAN_CPPON_OBJ_TYPE:
            // cppcheck-suppress cstyleCast
            rtn = ( ((COBoolean *) this)->value() ) ? 1.0 : 0.0;
            break;

        default:
            break;
    }
    return rtn;
}

CppON *CppON::operator = ( CppON &val )
{
    typ = val.typ;
    switch( val.typ )
    {
        case INTEGER_CPPON_OBJ_TYPE:
            deleteData();
            switch( siz = val.size() )
            {
                case sizeof( char ):
                    data = new ( char );
                    *((char*) data) = ((COInteger *) &val )->charValue();
                    break;

                case sizeof( short ):
                    data = new ( short );
                    *((short*) data) = ((COInteger*) &val )->shortValue();
                    break;

                case sizeof( int ):
                    data = new ( int );
                    *((int*) data) = ((COInteger*) &val )->intValue();
                    break;

                default:
                    data = new ( long long );
                    *((long long*) data) = ((COInteger*) &val )->longValue();
                    break;
            }
            break;

        case DOUBLE_CPPON_OBJ_TYPE:
            if( data )
            {
                if( 0 < this->precision  && 16 >= this->precision )
                {
                    double t;
                    double pow_10 = 1;
                    double n = *((double*) (*this).data);
                    double d = ((CODouble *) &val )->doubleValue();
                    if( this->precision )
                    {
                        pow_10 = pow( 10.0, this->precision);
                        n *= pow_10;
                        d *= pow_10;
                    }
                    t = n - d;
                    if( 0.75 < t || -0.75 > t )
                    {
                        *((double*) (*this).data) = round( d )/ pow_10;
                    }

                } else {
                    *((double *) data) = ((CODouble *) &val )->doubleValue();
                }
            } else {
                data = new double;
                *((double *) data) = ((CODouble *) &val )->doubleValue();
            }
            break;

        case STRING_CPPON_OBJ_TYPE:
            deleteData();
            data = ( val.data ) ? new std::string( ( ( std::string *) val.data )->c_str() ): NULL;
            break;

        case NULL_CPPON_OBJ_TYPE:
            siz = val.siz;
            break;

        case BOOLEAN_CPPON_OBJ_TYPE:
            data = new(char);
            *( ( char *) data) = ((COBoolean *) &val )->value();
            siz = 1;
            break;

        case MAP_CPPON_OBJ_TYPE:
            {
                deleteData();
                siz = val.size();
                data = new map<string, CppON*>();
                map<string, CppON*> *th = (map<string, CppON*> *) data;
                COMap  *m = (COMap *) &val;
                // cppcheck-suppress postfixOperator
                for( std::vector<std::string>::iterator itr = m->order.begin(); m->order.end() != itr; itr++ )
                {
                    CppON *obj = m->findCaseElement( itr->c_str() );
                    if( obj )
                    {
                        order.push_back( string( *itr ) );
                        switch( obj->typ )
                        {
                            case INTEGER_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                th->insert( th->end(), pair< string, CppON* >( itr->c_str(), new COInteger( *((COInteger *)obj ) ) ) );
                                break;

                            case DOUBLE_CPPON_OBJ_TYPE:
                                {
                                    // cppcheck-suppress cstyleCast
                                    char p = ( (CODouble *)obj )->Precision();
                                    CODouble *d;
                                    // cppcheck-suppress cstyleCast
                                    th->insert( th->end(), pair< string, CppON* >( itr->c_str(), d = new CODouble( *( (CODouble *)obj ) ) ) );
                                    if( 0 <= p )
                                    {
                                        d->Precision( p );
                                    }
                                }
                                break;

                            case STRING_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                th->insert( th->end(), pair< string, CppON* >( itr->c_str(), new COString( *( (COString *)obj ) ) ) );
                                break;

                            case NULL_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                th->insert( th->end(), pair< string, CppON* >( itr->c_str(), new CONull( *( (CONull *)obj ) ) ) );
                                break;

                            case BOOLEAN_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                th->insert( th->end(), pair< string, CppON* >( itr->c_str(), new COBoolean( *( (COBoolean *)obj ) ) ) );
                                break;

                            case MAP_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                th->insert( th->end(), pair< string, CppON* >( itr->c_str(), new COMap( *( (COMap *)obj ) ) ) );
                                break;

                            case ARRAY_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                th->insert( th->end(), pair< string, CppON* >( itr->c_str(), new COArray( *( (COArray *)obj ) ) ) );
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
            break;
        case ARRAY_CPPON_OBJ_TYPE:
            {
                deleteData();
                siz = val.size();
                data = new vector<CppON *>();
                for( int i = 0; siz > i; i++ )
                {
                    CppON *jt = ((COArray &) val ).at( i );
                    switch( jt->type() )
                    {
                        case INTEGER_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            ((COArray*)this)->append( new COInteger( *((COInteger *)jt ) ) );
                            break;

                        case DOUBLE_CPPON_OBJ_TYPE:
                            {
                                // cppcheck-suppress cstyleCast
                                char p = ( (CODouble *)jt )->Precision();
                                CODouble *d;
                                // cppcheck-suppress cstyleCast
                                ((COArray*)this)->append( d = new CODouble( *( (CODouble *)jt ) ) );
                                if( 0 <= p )
                                {
                                    d->Precision( p );
                                }
                            }
                            break;

                        case STRING_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            ((COArray*)this)->append( new COString( *( (COString *)jt ) ) );
                            break;

                        case NULL_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            ((COArray*)this)->append( new CONull( *( (CONull *)jt ) ) );
                            break;

                        case BOOLEAN_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            ((COArray*)this)->append( new COBoolean( *( (COBoolean *)jt ) ) );
                            break;

                        case MAP_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            ((COArray*)this)->append( new COMap( *( (COMap *)jt ) ) );
                            break;

                        case ARRAY_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            ((COArray*)this)->append( new COArray( *( (COArray *)jt ) ) );
                            break;

                        default:
                            break;
                    }
                }
            }
            break;
        default:
            break;
    }

    return this;
}

/****************************************************************************************/
/*                                                                                      */
/*                                         COBoolean                                    */
/*                                                                                      */
/****************************************************************************************/

string *COBoolean::toNetString()
{
    if( !data )
    {
        return NULL;
    }
    const char *val = *( (char *) data )  ? "true" : "false";
    return CppON::toNetString( val, '!' );
}

string *COBoolean::toJsonString()
{
    return new string( ((value())? "true" : "false") );
}

void COBoolean::dump( FILE *fp )
{
    fprintf( fp, "%s", (data) ? ( ( *(char *)data ) ? "true":"false") : "NULL" );
}
void COBoolean::cdump( FILE *fp )
{
    fprintf( fp, "%s", (data) ? ( ( *(char *)data ) ? "true":"false") : "false" );
}
const char  *COBoolean::c_str()
{
    str = ( data && *( (char *) data ))? "true" : "false";
    return str.c_str();
}

/****************************************************************************************/
/*                                                                                      */
/*                                    COMap                                             */
/*                                                                                      */
/****************************************************************************************/

COMap::COMap( COMap *mt ) : CppON(  MAP_CPPON_OBJ_TYPE )
{
    data = new map<string, CppON*>();
    std::map< std::string, CppON * >  &dm = *( ( map<string, CppON*> * ) data );

    // cppcheck-suppress postfixOperator
    for( std::vector<std::string>::iterator itr = mt->order.begin(); mt->order.end() != itr; itr++ )
    {
        CppON *obj = mt->findCaseElement( itr->c_str() );
        order.push_back( string( *itr ) );
        switch( obj->type() )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                dm[ *itr ] = new COInteger( *((COInteger *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
             case DOUBLE_CPPON_OBJ_TYPE:
                 dm[ *itr ] = new CODouble( *( (CODouble *)obj ) );
                 // cppcheck-suppress cstyleCast
                 break;
             case STRING_CPPON_OBJ_TYPE:
                 dm[ *itr ] = new COString( *( (COString *)obj ) );
                 // cppcheck-suppress cstyleCast
                 break;

             case NULL_CPPON_OBJ_TYPE:
                 dm[ *itr ] = new CONull( *( (CONull *)obj ) );
                 // cppcheck-suppress cstyleCast
                 break;

             case BOOLEAN_CPPON_OBJ_TYPE:
                 dm[ *itr ] = new COBoolean( *( (COBoolean *)obj ) );
                 // cppcheck-suppress cstyleCast
                 break;
             case MAP_CPPON_OBJ_TYPE:
                 dm[ *itr ] = new COMap( *( (COMap *)obj ) );
                 // cppcheck-suppress cstyleCast
                 break;
             case ARRAY_CPPON_OBJ_TYPE:
                 dm[ *itr ] = new COArray( *( (COArray *)obj ) );
                 // cppcheck-suppress cstyleCast
                 break;
             default:
                 break;
        }
    }
}

COMap::COMap( COMap & mt ) : CppON(  MAP_CPPON_OBJ_TYPE )
{
    data = new map<string, CppON*>();
    std::map< std::string, CppON * >  &dm = *( ( map<string, CppON*> * ) data );

    // cppcheck-suppress postfixOperator
    for( std::vector<std::string>::iterator itr = mt.order.begin(); mt.order.end() != itr; itr++ )
    {
        CppON *obj = mt.findCaseElement( itr->c_str() );
        order.push_back( string( *itr ) );
        switch( obj->type() )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                dm[ *itr ] = new COInteger( *((COInteger *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                dm[ *itr ] = new CODouble( *( (CODouble *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            case STRING_CPPON_OBJ_TYPE:
                dm[ *itr ] = new COString( *( (COString *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            case NULL_CPPON_OBJ_TYPE:
                dm[ *itr ] = new CONull( *( (CONull *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            case BOOLEAN_CPPON_OBJ_TYPE:
                dm[ *itr ] = new COBoolean( *( (COBoolean *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            case MAP_CPPON_OBJ_TYPE:
                dm[ *itr ] = new COMap( *( (COMap *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            case ARRAY_CPPON_OBJ_TYPE:
                dm[ *itr ] = new COArray( *( (COArray *)obj ) );
                // cppcheck-suppress cstyleCast
                break;
            default:
                break;
        }
    }
};

void COMap::parseData( const char *str )
{
    if( str )
    {
        json_t        *root = NULL;
        json_error_t  error;
        char          *np;
        int           cnt = strtol( str, &np, 0 );      // Check to see if it is a tnetstring (Starts with number)
        string        tabs( "" );

        if( np != str )
        {
            np++;
            if( cnt && ',' == np[ cnt ] )
            {
                char *tmp = strdup( np );                // make a copy so it can be modified
                tmp[ cnt ] = '\0';                      // remove the ',' at  the end of it
                root = json_loads( tmp, 0, &error );    // load it as JSON
                free( tmp );
            } else if ( cnt ) {
                root = json_loads( np, 0, &error );      // load it as JSON
            }
        }
        if( !root )                                  // I guess it wasn't tnetstring so load it as JSON
        {
            if( !( str && str[ 0 ] ) )
            {
                fprintf( stderr, "%s[%d] Error: Attempt to parse zero length JSON string\n",__FILE__, __LINE__ );
            }
            root = json_loads( str, 0, &error );
        }
        if( ! root  )
        {
            fprintf( stderr, "%s[%d] Error: on line %d %s\n", __FILE__, __LINE__, error.line, error.text );
            fprintf( stderr, "%s\n",str );
        }
        if( json_is_object( root ) )
        {
            const char *key;
            json_t   *value;
            json_object_foreach( root, key, value )
            {
                //  order.push_back( string( key ) );
                this->append( key, parseJson( value, tabs ) );
            }
        }
        json_decref( root );
    }
}

COMap::COMap( const char *path, const char *file ): CppON( MAP_CPPON_OBJ_TYPE )
{
    data = new map<string, CppON*>();
	struct stat     _stat;
	std::string p( path );
	FILE    *fp;

	if( '/' != p.back() )
	{
		p += '/';
	}
	p.append( file );
	if( ! stat( p.c_str(), &_stat ) && ! ( _stat.st_mode & DIRECTORY_BIT ) && (fp = fopen( p.c_str(), "r" ) ) )
	{
	    int     sz = 1024;
	    char    *buf,*bSave;
	    int     rd = 0;
	    int     c;

	    if( !( buf = (char *) malloc( sz ) ) )
	    {
	        fclose( fp );
	        fprintf( stderr, "%s[%.4u]: Failed to allocate memory",__FILE__, __LINE__ );
	    }
	    while( EOF != ( c = fgetc( fp ) ) )
	    {
	        buf[ rd++ ] = ( char ) c;
	        if( rd == sz )
	        {
	            if( !( buf = (char *) realloc( (void *) (bSave = buf ), sz += 512 ) ) )
	            {
	    	        fprintf( stderr, "%s[%.4u]: Failed to reallocate memory: %d bytes!",__FILE__, __LINE__, sz );
	                free( bSave );
	            }
	        }
	    }
	    buf[ rd ] = '\0';
	    fclose( fp );

	    parseData( buf );

	    free(buf );
	} else {
        fprintf( stderr, "%s[%.4u]: Failed to open JSON FILE \"%s\"",__FILE__, __LINE__, p.c_str() );
	}
}

COMap::COMap( const char *str ): CppON(  MAP_CPPON_OBJ_TYPE )
{
    data = new map<string, CppON*>();
#if 0
    parseData( str );
#else
    if( str )
    {
        json_t        *root = NULL;
        json_error_t  error;
        char          *np;
        int           cnt = strtol( str, &np, 0 );      // Check to see if it is a tnetstring (Starts with number)
        string        tabs( "" );

        if( np != str )
        {
            np++;
            if( cnt && ',' == np[ cnt ] )
            {
                char *tmp = strdup( np );                // make a copy so it can be modified
                tmp[ cnt ] = '\0';                      // remove the ',' at  the end of it
                root = json_loads( tmp, 0, &error );    // load it as JSON
                free( tmp );
            } else if ( cnt ) {
                root = json_loads( np, 0, &error );      // load it as JSON
            }
        }
        if( !root )                                  // I guess it wasn't tnetstring so load it as JSON
        {
            if( !( str && str[ 0 ] ) )
            {
                fprintf( stderr, "%s[%d] Error: Attempt to parse zero length JSON string\n",__FILE__, __LINE__ );
            }
            root = json_loads( str, 0, &error );
        }
        if( ! root  )
        {
            fprintf( stderr, "%s[%d] Error: on line %d %s\n", __FILE__, __LINE__, error.line, error.text );
            fprintf( stderr, "%s\n",str );
        }
        if( json_is_object( root ) )
        {
            const char *key;
            json_t   *value;
            json_object_foreach( root, key, value )
            {
                //  order.push_back( string( key ) );
                this->append( key, parseJson( value, tabs ) );
            }
        }
        json_decref( root );
    }
#endif
}

COMap *COMap::operator=( const char *str )
{
    if( str )
    {
        json_t                  *root = NULL;
        json_error_t            error;
        char                    *np;
        int                     cnt = strtol( str, &np, 0 );      // Check to see if it is a tnetstring (Starts with number)
        string                  tabs( "" );

        if( data )
        {
            map <string, CppON*> *m = ( map <string, CppON *> * ) data;
            map<string, CppON *>::iterator it;
            // cppcheck-suppress postfixOperator
            for( it = m->begin(); m->end() != it; it++ )
            {
                delete( it->second);
            }
            m->clear();
        } else {
            data = new map<string, CppON*>();
        }
        order.clear();
        if( np != str )
        {
            np++;
            if( cnt && ',' == np[ cnt ] )
            {
                char *tmp = strdup( np );                // make a copy so it can be modified
                tmp[ cnt ] = '\0';                      // remove the ',' at  the end of it
                root = json_loads( tmp, 0, &error );    // load it as JSON
                free( tmp );
            } else if ( cnt ) {
                root = json_loads( np, 0, &error );      // load it as JSON
            }
        }
        if( !root )                                  // I guess it wasn't tnetstring so load it as JSON
        {
            if( !( str && str[ 0 ] ) )
            {
                fprintf( stderr, "%s[%d] Error: Attempt to parse zero length JSON string\n",__FILE__, __LINE__ );
            }
            root = json_loads( str, 0, &error );
        }
        if( ! root  )
        {
            fprintf( stderr, "%s[%d] Error: on line %d %s\n", __FILE__, __LINE__, error.line, error.text );
            fprintf( stderr, "%s\n",str );
        }
        if( json_is_object( root ) )
        {
            const char *key;
            json_t   *value;
            json_object_foreach( root, key, value )
            {
                //  order.push_back( string( key ) );
                this->append( key, parseJson( value, tabs ) );
            }
        }
        json_decref( root );
    } else {
        if( data )
        {
            (( map <string, CppON *> * ) data)->clear();
        } else {
            data = new map<string, CppON*>();
        }
        order.clear();
    }
    return this;
}

// cppcheck-suppress unusedFunction
void COMap::replaceObj( string s, CppON *obj )
{
    map <string, CppON *> *m = ( map <string, CppON*> *) data;
    map <string, CppON *>::iterator it;
    if( m->end( ) != (it = m->find( s ) ) )
    {
        delete it->second;
        it->second = obj;
    }
}

// cppcheck-suppress unusedFunction
void COMap::removeVal( string s )
{
    map <string, CppON *> *m = ( map <string, CppON*> *) data;
    map <string, CppON *>::iterator it;

    if( m->end( ) != (it = m->find( s ) ) )
    {
        m->erase( it );
        for( std::vector<std::string>::iterator iter = order.begin(); order.end() != iter; ++iter )
        {
            if( ! iter->compare( s ) )
            {
                order.erase( iter );
                break;
            }
        }
    }
}

void COMap::clear( )
{
    map <string, CppON*> *m = ( map <string, CppON *> * ) data;
    map<string, CppON *>::iterator it;
    // cppcheck-suppress postfixOperator
    for( it = m->begin(); m->end() != it; it++ )
    {
        delete( it->second);
    }
    m->clear();
    order.clear();
}

void COMap::merge( COMap *targetObj, const char *name )
{
    if( ! data )
    {
        data = new std::map<std::string, CppON*>();                         		// data object you are merging too.
    }
    map<string, CppON *> *s = (map<string, CppON *> *) targetObj->data; 			// s is the data object of the map you are merging
    for( map< string, CppON *>::iterator ti = s->begin(); s->end() != ti; ++ti )	// foreach object in map you are merging
    {
        const string                      *targetStr = &(ti->first);      			// get Object name
        CppONType                       _eType = (ti->second)->type();      		// get type of object
        CppON                           *myObj = NULL;                      		// Place to save an object that is found.
        map<string, CppON *>::iterator  it;                                 		// Used to search destination

        map<string, CppON *> *m = (map<string, CppON *> *) data;          			// Search the destination for any objects with the same name.
        // cppcheck-suppress postfixOperator
        for( it = m->begin(); m->end() != it; it++ )
        {
            if( ! strcmp(  (it->first).c_str(), targetStr->c_str() ) )          	// Find the object
            {
                myObj = it->second;                                               	// OK we have it already. set "myObj" to it and quit looking
                break;
            }
        }
        if( myObj )                                                           		// We found it so we need to merge the reasult
        {
            switch( _eType )                                                    	// The merge is handled differently depending on type of object it is
            {
                case INTEGER_CPPON_OBJ_TYPE:                                     	// If it is an integer
                    if( INTEGER_CPPON_OBJ_TYPE == myObj->type())                   	//    If the old object is the same type then we
                    {
                        // cppcheck-suppress cstyleCast
                        *((COInteger *) myObj) = (long long)(COInteger *)(ti->second )->toLongInt(); // set it to the new one
                    } else {                                                          //    If not
                        m->erase( it );                                                 //      Delete it and add the new object
                        delete myObj;
                        // cppcheck-suppress cstyleCast
                        append( targetStr->c_str(), new COInteger( (long long)(COInteger *)( ti->second )->toLongInt( ) ) );
                    }
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:                                             //  If it is a double (like an integer )
                    if( DOUBLE_CPPON_OBJ_TYPE == myObj->type())                           //      If the old object is also a double
                    {
                        // cppcheck-suppress cstyleCast
                        ((CODouble *) myObj)->set( ((CODouble *)(ti->second ) )->toDouble());  // set it to the new value
                    } else {                                                          //      else if it of a different type than delete the old and replace
                        m->erase( it );
                        delete myObj;
                        // cppcheck-suppress cstyleCast
                        append( targetStr->c_str(), new CODouble( (double)((CODouble *)( ti->second ))->toLongInt( ) ) );
                    }
                    break;
                case STRING_CPPON_OBJ_TYPE:                                               // Strings are treated like integers and doubles
                    if( STRING_CPPON_OBJ_TYPE == myObj->type())
                    {
                        // cppcheck-suppress cstyleCast
                        *((COString *) myObj) = ((COString *)(ti->second ) )->c_str();
                    } else {
                        m->erase( it );
                        delete myObj;
                        // cppcheck-suppress cstyleCast
                        append( targetStr->c_str(), new COString( ((COString *)( ti->second ))->c_str() ) );
                    }
                    break;
                case NULL_CPPON_OBJ_TYPE:                                                 //  In the case of null, it doesn't have a value
                    if( NULL_CPPON_OBJ_TYPE != myObj->type())                               //  But if the old one exist and it isn't null,
                    {
                        m->erase( it );                                                   //    delete it and set it to null
                        delete myObj;
                        append( targetStr->c_str(), new CONull() );
                    }
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:                                              // booleans are treated like integers, doubles and strings
                    if( BOOLEAN_CPPON_OBJ_TYPE == myObj->type())
                    {
                        // cppcheck-suppress cstyleCast
                        *((COBoolean *) myObj) = *((COBoolean *)(ti->second ));
                    } else {
                        m->erase( it );
                        delete myObj;
                        // cppcheck-suppress cstyleCast
                        append( targetStr->c_str(), new COBoolean( ((COBoolean *)( ti->second ))->value() ) );
                    }
                    break;
                case  MAP_CPPON_OBJ_TYPE:
                    {
                        // cppcheck-suppress cstyleCast
                        COMap     *pt = ( COMap *) ti->second;
                        if( MAP_CPPON_OBJ_TYPE == myObj->type())                              //    IF the old one is also a map then
                        {
                            // cppcheck-suppress cstyleCast
                            ((COMap *)it->second )->merge( pt , name );                     //      merge the two
                        } else {                                                          //    else
                            m->erase( it );                                                 //      delete the old one and merge the new one.
                            delete myObj;
                            append( targetStr->c_str(), new COMap( *pt ) );
                        }
                    }
                    break;
                case  ARRAY_CPPON_OBJ_TYPE:
                    {
                        // cppcheck-suppress cstyleCast
                        COArray   *arrTarget = (COArray *) ti->second;
                        for( int k = 0; arrTarget->size() > k; k++ )
                        {
                            COString  *str;
                            COString  *namePtr;
                            COMap     *tMap;
                            // cppcheck-suppress cstyleCast
                            if( CppON::isMap( tMap = (COMap *)arrTarget->at( k ) ) && CppON::isString( namePtr = (COString *) (tMap->findElement( name ) ) ) )
                            {
                                // cppcheck-suppress cstyleCast
                                COArray   *arr = (COArray *) it->second;
                                int       i;
                                for( i = 0; arr->size() > i; i++ )
                                {
                                    COMap     *uMap;
                                    // cppcheck-suppress cstyleCast
                                    if( CppON::isMap( uMap = (COMap *)arr->at( i ) ) && CppON::isString( str = (COString *) (uMap->findElement( name ) ) ) && !strcmp( str->c_str(), namePtr->c_str() ) )
                                    {
                                        uMap->merge( tMap, name );
                                        break;
                                    }
                                }
                                if( arr->size() <= i )
                                {
                                    arr->append( new COMap( *tMap ));
                                }
                                // cppcheck-suppress cstyleCast
                            } else if( CppON::isString( str = (COString *)arrTarget->at( k ) ) ) {
                                // cppcheck-suppress cstyleCast
                                COArray   *arr = (COArray *) myObj;
                                int       k1;

                                for( k1 = 0; arr->size() > k1; k1++ )                                 // Check if a string exists in target object that is the same
                                {
                                    // cppcheck-suppress cstyleCast
                                    COString  *kStr = (COString *) arr->at( k1 );
                                    if( CppON::isString( kStr ) )
                                    {
                                        if( ! strcmp( str->c_str(), kStr->c_str() ) )
                                        {
                                            break;
                                        }
                                    }
                                }
                                if( arr->size() == k1 )
                                {
                                    arr->append( new COString( *str ) );
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        } else {                                                                        // It doesn't have this so just add it.
            // cppcheck-suppress cstyleCast
            append( *targetStr, CppON::factory( *(( CppON *)ti->second )) );
        }
    }
}

void COMap::upDate( COMap *target, const char *name )
{
    if( data )
    {
        map<string, CppON *> *s = (map<string, CppON *> *) target->data;

        for( map< string, CppON *>::iterator ti = s->begin(); s->end() != ti; ++ti )
        {
            bool                 				found = false;
            map<string, CppON *>::iterator   	it;
            const string             			*targetStr = &(ti->first);
            CppONType              				_eType = (ti->second)->type();

            map<string, CppON *> *m = (map<string, CppON *> *) data;
            // cppcheck-suppress postfixOperator
            for( it = m->begin(); m->end() != it; it++ )
            {
                if( ! strcmp(  (it->first).c_str(), targetStr->c_str() ) )
                {
                    found = true;
                    if( (it->second)->type() == _eType )                                              // If they are the same data type then just update it
                    {
                        switch( _eType )
                        {
                            case INTEGER_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                *((COInteger *)it->second ) =  ( (COInteger *)ti->second )->intValue();
                                break;

                            case DOUBLE_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                *((CODouble *)it->second ) =  ( (CODouble *)ti->second )->doubleValue();
                                break;

                            case STRING_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                *((COString *)it->second ) = ( (COString *)ti->second )->c_str() ;
                                break;

                            case BOOLEAN_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                *((COBoolean *)it->second ) = ( (COBoolean *)ti->second )->value();
                                break;

                            case  MAP_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                ((COMap *)it->second )->upDate( ( (COMap *) ti->second ), name );
                                break;

                            case  ARRAY_CPPON_OBJ_TYPE:
                                {
                                    // cppcheck-suppress cstyleCast
                                    COArray   *arrTarget = (COArray *) ti->second;
                                    for( int k = 0; arrTarget->size() > k; k++ )
                                    {
                                        COString *namePtr;
                                        COMap *tMap;
                                        // cppcheck-suppress cstyleCast
                                        if( CppON::isMap( tMap = (COMap *)arrTarget->at( k ) ) && CppON::isString( namePtr = (COString *) (tMap->findElement( name ) ) ) )
                                        {
                                            // cppcheck-suppress cstyleCast
                                            COArray     *arr = (COArray *) it->second;
                                            for( int i = 0; arr->size() > i; i++ )
                                            {
                                                COString *str;
                                                COMap     *uMap;
                                                // cppcheck-suppress cstyleCast
                                                if( CppON::isMap( uMap = (COMap *)arr->at( i ) ) && CppON::isString( str = (COString *) (uMap->findElement( name ) ) ) && !strcmp( str->c_str(), namePtr->c_str() ) )
                                                {
                                                	COMap *newMap = new COMap( *tMap );
                                                    if( ! arr->replace( i, newMap ) )
                                                    {
                                                    	delete newMap;
                                                    }
                                                    delete uMap;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                break;

                            case NULL_CPPON_OBJ_TYPE:
                                fprintf( stderr, "COMap:update - Null 1st being ignored\n");

                                break;
                            default:
                                break;
                        }
                    } else {
                        delete ( it->second );                                                                          // replace the data type with the new one.
                        switch( _eType )
                        {
                            case INTEGER_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                it->second =  new COInteger( ( (COInteger *)ti->second )->intValue() );
                                break;

                            case DOUBLE_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                it->second =  new CODouble( ( (CODouble *)ti->second )->doubleValue() );
                                break;

                            case STRING_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                it->second = new COString( ( (COString *)ti->second )->c_str() );
                                break;

                            case BOOLEAN_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                it->second = new COBoolean( ( (COBoolean *)ti->second )->value() );
                                break;

                            case  MAP_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                it->second  = new COMap( * ( (COMap *)ti->second ) );
                                break;

                            case  ARRAY_CPPON_OBJ_TYPE:
                                // cppcheck-suppress cstyleCast
                                it->second  = new COArray( * ( (COArray *)ti->second ) );
                                break;

                            case NULL_CPPON_OBJ_TYPE:
                                it->second = new CONull();
                                fprintf( stderr, "COMap:update - Null being ignored\n");
                                target->dump();
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
            if( ! found )
            {
                switch ( _eType )
                {
                    case INTEGER_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        this->append( ti->first, new COInteger( *((COInteger *) ti->second) ) );
                        break;

                    case DOUBLE_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        this->append( ti->first, new CODouble( *((CODouble *) ti->second) ) );
                        break;

                    case STRING_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        this->append( ti->first, new COString( *((COString *) ti->second) ) );
                        break;

                    case BOOLEAN_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        this->append( ti->first, new COBoolean( *((COBoolean *) ti->second) ) );
                        break;

                    case NULL_CPPON_OBJ_TYPE:
                        this->append( ti->first, new CONull() );
                        break;

                    case  MAP_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        this->append( ti->first, new COMap( *((COMap *) ti->second) ) );
                        break;

                    case  ARRAY_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        this->append( ti->first, new COArray( *((COArray *) ti->second) ) );
                        break;

                    default:
                        break;
                }
            }
        }
    }
}

CppON *COMap::findEqual( const char * name, CppON &search )
{
    CppON     *rtn = NULL;
    CppONType    _eType = search.type();

    if( data )
    {
        map<string, CppON *>::iterator it;
        map<string, CppON *> *m = (map<string, CppON *> *) data;
        // cppcheck-suppress postfixOperator
        for( it = m->begin(); !rtn && m->end() != it; it++ )
        {
            if( ! strcmp(  (it->first).c_str(), name ) && (it->second)->type() == _eType )
            {
                switch( _eType )
                {
                    case INTEGER_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( ((COInteger &) search).intValue() == ((COInteger *)it->second )->intValue() )
                            rtn = (it->second);
                        break;

                    case DOUBLE_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( ((CODouble &) search).doubleValue() == ((CODouble *)it->second )->doubleValue() )
                            rtn = (it->second);
                        break;

                    case STRING_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( !strcmp( ( ( COString &) search).c_str(), ((COString *)it->second )->c_str() ) )
                            rtn = ( it->second );
                        break;

                    case BOOLEAN_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( ((COBoolean &) search).value() == ((COBoolean *)it->second )->value() )
                            rtn = (it->second);
                        break;

                    case NULL_CPPON_OBJ_TYPE:
                        rtn = (it->second );
                        break;

                    default:
                        break;
                }
            }
            if( ! rtn )
            {
                if( MAP_CPPON_OBJ_TYPE == (it->second)->type() )
                {
                    // cppcheck-suppress cstyleCast
                    rtn = ( ( COMap *) it->second )->findEqual( name, search );
                } else if( ARRAY_CPPON_OBJ_TYPE == (it->second)->type() ) {
  //            rtn = ( ( COArray *) it->second )->findEqual( name, search );
                }
            }
        }
    }

    return rtn;
}

/*
 * Splits a path based on the forward slash ('/') into a val string and a remainder part
 * basically on retruning everything before the slash is in the "val" string and the remainder points to everything after the slash
 *
 */

static void splitPath( const char *str, std::string &val, const char **remainder )
{
	if( str && remainder )
	{
		for( *remainder = str; *(*remainder) && '/' != *(*remainder) ; (*remainder)++ );

		val = std::string( str, ( *remainder - str ) );

		if( '/' == *(*remainder) )
		{
			(*remainder)++;
		}
	}
}

/*
 * This and its companion "findCaseElement" if the typical way to find object in a map.
 *
 * Normally we are looking for an element directly in a structure but if we are need to go further
 * and retrieve an object in a Map thats in the Map we can deleminate the levels with a forward slash.
 * Likewise if we are looking for an element in an array  then we can delimit that with a colon.  So
 *  config/axisEncoders:2/resolution would search the structure to get the "config" map which it would search
 *  to get the axisEncoders array and retrieve the third element retruning the resolution object of it.
 *  {
 *    config: {
 *      axisEncoders: [
 *        {
 *        },
 *        {
 *        },
 *        {
 *          name: \"Gurely\",
 *          resolution:0.01
 *        }
 *      ]
 *    }
 *  }
 *
 */

CppON *COMap::findElement( const char *str )
{
    CppON *rtn = NULL;
    if( data && str )
    {
        int         arrayIndex = -1;

        /*
         * Split the path with s = to anything before a forward slash ('/') and str pointing to first character after it.
         */
        std::string s;
        splitPath( str, s, &str );                                // split the string based on a slash

        /*
         * Now look to see if there is an index after the original part denoted by a colon (':')
         */
        size_t idx = s.find( ':' );
        std::string more;
        if( std::string::npos != idx )                              // colon is found
        {
            char *r;
            arrayIndex = strtol( &s.c_str()[ idx + 1 ], &r, 10 );     // Read the index as a base 10 number following the colon
            more = r;                                                 // more will contain any text after the number
            s = s.substr( 0, idx );                                   // s only contains the text up to the colon
        }

        /*
         * OK search the map for an item with a name matching the text in 's'
         */

        map<string, CppON *> *m = (map<string, CppON *> *) data;
        // cppcheck-suppress postfixOperator
        for( map<string, CppON *>::iterator it = m->begin(); m->end() != it; it++ )
        {
            if( ! strcmp(  (it->first).c_str(), s.c_str() ) )         	// if found set "rtn" to the object and quit looking
            {
                rtn = it->second;
                break;
            }
        }

        if( rtn && CppON::isMap( rtn ) && *str )               		// But wait! if the object found is a map and there is a remainder from the
        {                                                           	// original split then call ourselves with the new string to find the object
            // cppcheck-suppress cstyleCast
            rtn = ( (COMap *) rtn )->findElement( str );
        } else if( rtn && CppON::isArray( rtn ) && 0 <= arrayIndex ) {    	// Or maybe the returned object was an array and we have an index
            while( CppON::isArray( rtn ) && 0 <= arrayIndex )
            {
                // cppcheck-suppress cstyleCast
                rtn = ( (COArray*) rtn)->at( arrayIndex );
                if( CppON::isMap( rtn ) && *str )
                {
                    // cppcheck-suppress cstyleCast
                    rtn = ( (COMap *) rtn)->findElement( str );
                    break;
                } else if( ! CppON::isArray( rtn ) || more[ 0 ] != ':' ) {
                    break;
                } else {
                    char *r;
                    s = more.substr( 1 );
                    arrayIndex = strtol( s.c_str(), &r, 10 );
                    more = r;
                }
            }
        }
    }
    return rtn;
}
// cppcheck-suppress unusedFunction
CppON *COMap::findNoSplit( const char *str )
{
    CppON *rtn = NULL;
    if( data && str )
    {
        /*
         * OK search the map for an item with a name matching the text in 's'
         */
        map<string, CppON *> *m = (map<string, CppON *> *) data;
        // cppcheck-suppress postfixOperator
        for( map<string, CppON *>::iterator it = m->begin(); m->end() != it; it++ )
        {
            if( ! strcmp( (it->first).c_str(), str ) )         // if found set "rtn" to the object and quit looking
            {
                rtn = it->second;
                break;
            }
        }
    }
    return rtn;
}
CppON *COMap::findCaseElement( const char *str )
{
    CppON *rtn = NULL;
    if( data  && str )
    {
        /*
         * Split the path with s = to anything before a forward slash ('/') and str pointing to first character after it.
         */

        int         arrayIndex = -1;
        std::string s;
        splitPath( str, s, &str );

        /*
         * Now look to see if there is an index after the original part denoted by a colon (':')
         */
        size_t idx = s.find( ':' );
        std::string more;
        if( std::string::npos != idx )                              // colon is found
        {
            char *r;
            arrayIndex = strtol( &s.c_str()[ idx + 1 ], &r, 10 );     // Read the index as a base 10 number following the colon
            more = r;                                                 // more will contain any text after the number
            s = s.substr( 0, idx );                                   // s only contains the text up to the colon
        }

        map<string, CppON *> *m = (map<string, CppON *> *) data;
        // cppcheck-suppress postfixOperator
        for( map<string, CppON *>::iterator it = m->begin(); m->end() != it; it++ )
        {
            if( ! strcasecmp(  (it->first).c_str(), s.c_str() ) )
            {
                rtn = it->second;
                break;
            }
        }
        if( rtn && CppON::isMap( rtn ) && *str )
        {
            // cppcheck-suppress cstyleCast
            rtn = ( (COMap *) rtn )->findCaseElement( str );
        } else if(CppON::isArray( rtn ) && 0 <= arrayIndex ) {    // Or maybe the returned object was an array and we have an index
            while( CppON::isArray( rtn ) && 0 <= arrayIndex )
            {
                // cppcheck-suppress cstyleCast
                rtn = ( (COArray*) rtn)->at( arrayIndex );
                if( CppON::isMap( rtn ) && *str )
                {
                    // cppcheck-suppress cstyleCast
                    rtn = ( (COMap *) rtn)->findElement( str );
                    break;
                } else if( ! CppON::isArray( rtn ) || more[ 0 ] != ':' ) {
                    break;
                } else {
                    char *r;
                    s = more.substr( 1 );
                    arrayIndex = strtol( s.c_str(), &r, 10 );
                    more = r;
                }
            }
        }
    }
    return rtn;
}
std::string *COMap::toCompactJsonString( )
{
    std::string *rtn = new string( "{" );
    if( data )
    {
        map<string, CppON *>::iterator   it;
        map<string, CppON *>            *m = (map<string, CppON *> *) data;
        CppON                           *n;
        std::string                       *sptr;
        bool                              first = true;

        for( size_t idx = 0; order.size() > idx; ++idx )
        {
            it = m->find( order.at( idx ) );
            if( first )
            {
                first = false;
            } else {
                rtn->append( "," );
            }
            *rtn += '\"';
            rtn->append( ( ( string ) it->first ).c_str() );
            rtn->append( "\":" );
            // cppcheck-suppress cstyleCast
            n = (CppON *) it->second;
            sptr = NULL;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COInteger *) n )->toJsonString();
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (CODouble *) n )->toJsonString();
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COString *) n )->toJsonString();
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COBoolean *) n )->toJsonString();
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = (( CONull *) n )->toJsonString();
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COMap *) n )->toCompactJsonString( );
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COArray *) n )->toCompactJsonString( );
                    break;
                default:
                    break;
            }
            if( sptr )
            {
                *rtn += *sptr;
                delete sptr;
            }
        }
    }
    *rtn += '}';
    return rtn;
}

std::string *COMap::toJsonString( std::string &indent )
{
    std::string *rtn = new string( indent.c_str() );
    rtn->append( "{\n" );
    if( data )
    {
        map<string, CppON *>::iterator   it;
        map<string, CppON *>            *m = (map<string, CppON *> *) data;
        CppON                           *n;
        std::string                       *sptr;
        bool                              first = true;
        std::string                       newIndent = indent;

        newIndent.append( "  " );
        for( std::vector< std::string >::iterator ito = order.begin(); order.end() != ito; ito++ )
        {
            it = m->find( *ito );
            if( first )
            {
                first = false;
            } else {
                rtn->append( ",\n" );
            }
            rtn->append( newIndent.c_str() );
            *rtn += '\"';
            rtn->append( ( ( string ) it->first ).c_str() );
            rtn->append( "\": " );
            // cppcheck-suppress cstyleCast
            n = (CppON *) it->second;
            sptr = NULL;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COInteger *) n )->toJsonString();
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (CODouble *) n )->toJsonString();
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COString *) n )->toJsonString();
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COBoolean *) n )->toJsonString();
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = (( CONull *) n )->toJsonString();
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    rtn->append( "\n" );
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COMap *) n )->toJsonString( newIndent );
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    rtn->append( "\n" );
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COArray *) n )->toJsonString( newIndent );
                    break;
                default:
                    break;
            }
            if( sptr )
            {
                *rtn += *sptr;
                delete sptr;
            }
        }
    }
    *rtn += '\n';
    rtn->append( indent.c_str() );
    *rtn += '}';
    return rtn;
}

string *COMap::toNetString()
{
    if( data && MAP_CPPON_OBJ_TYPE == typ )
    {
        std::string rt;
        std::map<std::string, CppON *>::iterator it;
        std::map<std::string, CppON *> *m = (map<string, CppON *> *) data;

        for( size_t idx = 0; order.size() > idx; ++idx )
        {
            std::string *sptr;
            it = m->find( order.at( idx ) );
            rt += *( sptr = CppON::toNetString( ( ( string ) it->first).c_str(), ',' ) );
            delete ( sptr );
            // cppcheck-suppress cstyleCast
            CppON *n = (CppON *) it->second;
            sptr = NULL;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COInteger *) n )->COInteger::toNetString();
                    break;

                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (CODouble *) n )->CODouble::toNetString();
                    break;

                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COString *) n )->COString::toNetString();
                    break;

                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COBoolean *) n )->COBoolean::toNetString();
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = (( CONull *) n )->CONull:: toNetString();
                    break;

                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COMap *) n )->COMap::toNetString();
                    break;

                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COArray *) n )->COArray::toNetString();
                    break;

                default:
                    fprintf( stderr, "Map::toNetString: Unknown CppONType\n");
                    break;
            }
            if( sptr )
            {
                rt+= *sptr;
                delete sptr;
            }
        }
        return CppON::toNetString( rt.c_str(), '}' );
    }
    return NULL;
}

const char  *COMap::c_str( std::string &idnt )
{
    str = "{";
    if( data )
    {
        map<string, CppON *>::iterator  it;
        map<string, CppON *>            *m = (map<string, CppON *> *) data;
        const char                        *comma = "\n";
        std::string                       indent = idnt;

        for( size_t idx = 0; order.size() > idx; ++idx )
        {
            it = m->find( order.at( idx ) );
            str.append( comma );
            comma = ",\n";
            str.append( indent );
            str += '"';
            str += it->first;
            str.append( "\": " );
            // cppcheck-suppress cstyleCast
            CppON *n = (CppON *) it->second;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (COInteger *) n )->c_str();
                    break;

                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (CODouble *) n )->c_str();
                    break;

                case STRING_CPPON_OBJ_TYPE:
                    str += '"';
                    // cppcheck-suppress cstyleCast
                    str += ( ( COString *) n )->c_str();
                    str += '"';
                  break;

                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( ( COBoolean *) n )->c_str();
                    break;

                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( ( CONull *) n )->c_str();
                    break;

                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (COMap *) n )->c_str( indent );
                    break;

                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (COArray *) n )->c_str( indent );
                    break;

                default:
                    break;
            }
        }
        str += '\n';
    }
    str += '}';
    return str.c_str();
}


void COMap::dump( string &indent, FILE *fp )
{
    if( data )
    {
        map<string, CppON *>::iterator   it;
        map<string, CppON *>            *m = (map<string, CppON *> *) data;
        bool                              first = true;
        string                            newIndent = indent;

        newIndent.append( "\t" );

        fprintf( fp, "%s{",indent.c_str() );

        for( size_t idx = 0; order.size() > idx; ++idx )
        {
            it = m->find( order.at( idx ) );
            if( first )
            {
                fprintf( fp, "\n%s\"%s\": ", newIndent.c_str(), ( ( string ) it->first).c_str() );
                first = false;
            } else {
                fprintf( fp, ",\n%s\"%s\": ", newIndent.c_str(), ( ( string ) it->first).c_str() );
            }

            // cppcheck-suppress cstyleCast
            CppON *n = (CppON *) it->second;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (COInteger *) n )->dump( fp );
                    break;

                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (CODouble *) n )->dump( fp );
                    break;

                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( COString *) n )->dump( fp );
                    break;

                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( COBoolean *) n )->dump( fp );
                    break;

                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( CONull *) n )->dump( fp );
                    break;

                case MAP_CPPON_OBJ_TYPE:
                    fprintf( fp, "\n");
                    // cppcheck-suppress cstyleCast
                    ( (COMap *) n )->dump( newIndent, fp );
                    break;

                case ARRAY_CPPON_OBJ_TYPE:
                    fprintf( fp, "\n");
                    // cppcheck-suppress cstyleCast
                    ( (COArray *) n )->dump( newIndent, fp  );
                    break;

                default:
                    break;
            }
        }
        fprintf( fp, "\n%s}",indent.c_str() );
    } else {
        fprintf( fp, "NULL");
    }
}

void COMap::cdump( FILE *fp )
{
    if( data )
    {
        map<string, CppON *>::iterator   it;
        map<string, CppON *>            *m = (map<string, CppON *> *) data;
        bool                              first = true;
        fprintf( fp, "{" );

        for( size_t idx = 0; order.size() > idx; ++idx )
        {
            it = m->find( order.at( idx ) );
            if( first )
            {
                fprintf( fp, "\\\"%s\\\": ", ( ( string ) it->first).c_str() );
                first = false;
            } else {
                fprintf( fp, ",\\\"%s\\\": ", ( ( string ) it->first).c_str() );
            }

            // cppcheck-suppress cstyleCast
            CppON *n = (CppON *) it->second;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (COInteger *) n )->cdump( fp );
                    break;

                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (CODouble *) n )->cdump( fp );
                    break;

                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( COString *) n )->cdump( fp );
                    break;

                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( COBoolean *) n )->cdump( fp );
                    break;

                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( CONull *) n )->cdump( fp );
                    break;

                case MAP_CPPON_OBJ_TYPE:
                    fprintf( fp, "\"\n\"");
                    // cppcheck-suppress cstyleCast
                    ( (COMap *) n )->cdump( fp );
                    break;

                case ARRAY_CPPON_OBJ_TYPE:
                    fprintf( fp, "\"\n\"");
                    // cppcheck-suppress cstyleCast
                    ( (COArray *) n )->cdump( fp  );
                    break;

                default:
                    break;
            }
        }
        fprintf( fp, "}" );
    } else {
        fprintf( fp, "{}");
    }
}

/*
 * Given a path, write a file to disk
 */

int  COMap::toFile( const char *path )
{
    FILE    *fp = NULL;
    int     rtn = 0;
    /*
     * Make sure we were passed something as a path and if so attempt to open it for writing
     */
    if( path && *path && (fp = fopen( path, "w" ) ) )
    {
        dump( fp );
        fclose( fp );

    } else {
        rtn = -1;
    }
    return rtn;
}

/*
 * Certain types take priority over others and so they are "promoted" to the higher priority.  They are ordered:
 *  1) Map or Array
 *  2) boolean
 *  3) double
 *  4) Integer
 *  5) String
 *  6) NULL;
 *
 *  The odd guy is the NULL. The NUll replaces everyone but everyone replaces the NULL
 */

static void appendTag( string name, CppON *obj, COMap *rtn, CppON *n )
{

    if( obj->type() == MAP_CPPON_OBJ_TYPE )
    {
     // cppcheck-suppress cstyleCast
        rtn->append( name, new COMap( *((COMap *) obj) ) );
    } else if( obj->type() == ARRAY_CPPON_OBJ_TYPE ) {
        // cppcheck-suppress cstyleCast
        rtn->append( name, new COArray( *((COArray *) obj) ) );
    } else {
        switch ( n->type() )
        {
            case BOOLEAN_CPPON_OBJ_TYPE:                                            // If original type was a boolean then promote everything to boolean except null;
                {
                    // cppcheck-suppress cstyleCast
                    bool v = ((COBoolean *) n )->value();
                    bool r;
                    switch( obj->type() )
                    {
                        case BOOLEAN_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ( r = ((COBoolean *) obj )->value() ) )
                            {
                                rtn->append( name, new COBoolean( r ) );
                            }
                            break;

                        case INTEGER_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ( r = ( ( (COInteger *) obj)->intValue() != 0 ) ) )
                            {
                                rtn->append( name, new COBoolean( r ) );
                            }
                            break;

                        case DOUBLE_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ( r = ( ( ( CODouble *) obj)->toInt() != 0 ) ) )
                            {
                                rtn->append( name, new COBoolean( r ) );
                            }
                            break;

                        case STRING_CPPON_OBJ_TYPE:
                            {
                                // cppcheck-suppress cstyleCast
                                const char *cPtr = ((COString *) obj)->c_str();
                                if( v != ( r = ( 0 != ( !strcasecmp( cPtr, "true") ) ? 1 : ( (!strcasecmp( cPtr, "false" ) ) ? 0 : strtol( cPtr, NULL, 10 ) ) ) ) )
                                {
                                    rtn->append( name, new COBoolean( r ) );
                                }
                            }
                            break;

                        case NULL_CPPON_OBJ_TYPE:
                            rtn->append( name, new CONull( ) );
                            break;
                        default:
                            break;
                    }
                }
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                {
                    // cppcheck-suppress cstyleCast
                    double v = ((CODouble *) n )->doubleValue();
                    double r;
                    switch( obj->type() )
                    {
                        case DOUBLE_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ((CODouble *) obj)->doubleValue() )
                            {
                                // cppcheck-suppress cstyleCast
                                rtn->append( name, new CODouble( *((CODouble *) obj) ) );
                            }
                            break;

                        case INTEGER_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ( r = (double)( ( COInteger * ) obj )->longValue() ) )
                            {
                                rtn->append( name, new CODouble( r ) );
                            }
                            break;

                        case STRING_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ( r = strtod( ((COString *) obj)->c_str(), NULL ) ) )
                            {
                                rtn->append( name, new CODouble( r ) );
                            }
                            break;

                        case BOOLEAN_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new COBoolean( *((COBoolean *) obj) ) );
                            break;

                        case NULL_CPPON_OBJ_TYPE:
                            rtn->append( name, new CONull( ) );
                            break;
                        default:
                            break;
                    }
                }
                break;

            case INTEGER_CPPON_OBJ_TYPE:
                {
                    // cppcheck-suppress cstyleCast
                    long long v = ((COInteger *) n)->toLongInt();
                    long long r;
                    switch( obj->type() )
                    {
                        case INTEGER_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ((COInteger *) obj)->toLongInt() )
                            {
                                // cppcheck-suppress cstyleCast
                                rtn->append( name, new COInteger( *((COInteger *) obj) ) );
                            }
                            break;

                        case STRING_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            if( v != ( r = strtol( ((COString *) obj)->c_str(), NULL, 0 ) ) )
                            {
                                rtn->append( name, new COInteger( r ) );
                            }
                            break;

                        case DOUBLE_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new CODouble( *((CODouble *) obj) ) );
                            break;
                        case BOOLEAN_CPPON_OBJ_TYPE:
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new COBoolean( *((COBoolean *) obj) ) );
                            break;
                        case NULL_CPPON_OBJ_TYPE:
                            rtn->append( name, new CONull( ) );
                            break;
                        default:
                            break;
                    }
                }
                break;

            default:
                switch( obj->type() )
                {
                    case INTEGER_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((COInteger *) n ) != *((COInteger *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new COInteger( *((COInteger *) obj) ) );
                        }
                        break;

                    case DOUBLE_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((CODouble *) n ) != *((CODouble *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new CODouble( *((CODouble *) obj) ) );
                        }
                        break;

                    case BOOLEAN_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((COBoolean *) n ) != *((COBoolean *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new COBoolean( *((COBoolean *) obj) ) );
                        }
                        break;

                    case NULL_CPPON_OBJ_TYPE:
                        if( n->type() != NULL_CPPON_OBJ_TYPE )
                        {
                            rtn->append( name, new CONull( ) );
                        }
                        break;

                    case STRING_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((COString *) n ) != *((COString *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( name, new COString( *((COString *) obj) ) );
                        }
                        break;

                    case MAP_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( name, new COMap( *((COMap *) obj) ) );
                        break;

                    case ARRAY_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( name, new COArray( *((COArray *) obj) ) );
                        break;

                    default:
                        break;
                }
                break;
        }
    }
}

COMap  *COMap::diff( COMap &newObj, const char *name )
{
    COMap                                    *rtn   = new COMap();
    CppON                                 *n;

    map<string, CppON *>::iterator        it;
    map<string, CppON *>                  *m     = (map<string, CppON *> *) data;

    // cppcheck-suppress postfixOperator
    for( it = m->begin(); m->end() != it; it++ )
    {
        CppON                *obj;
        if( ( obj = newObj.findElement( (string *)&(it->first) ) ) )
        {
            // cppcheck-suppress cstyleCast
            if( ( n = (CppON *) it->second ) )
            {
                COMap                  *nv;
                COArray                *na;
                switch ( n->type() )
                {
                    case INTEGER_CPPON_OBJ_TYPE:
                        appendTag( it->first, obj, rtn, n );
                        break;
                    case DOUBLE_CPPON_OBJ_TYPE:
                        appendTag( it->first, obj, rtn, n );
                        break;
                    case STRING_CPPON_OBJ_TYPE:
                        appendTag( it->first, obj, rtn, n );
                        break;
                    case BOOLEAN_CPPON_OBJ_TYPE:
                        appendTag( it->first, obj, rtn, n );
                        break;
                    case NULL_CPPON_OBJ_TYPE:
                        appendTag( it->first, obj, rtn, n );
                        break;
                    case MAP_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( (nv = ((COMap *) n )->diff( *((COMap *) obj ), name ) ) )
                        {
                            rtn->append( it->first, nv );
                        }
                        break;
                    case ARRAY_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( (na = ( (COArray *) n )->diff( *( ( COArray *) obj ), name ) ) )
                        {
                            rtn->append( it->first, na );
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    m = (map<string, CppON *> *) newObj.getData();
    // cppcheck-suppress postfixOperator
    for( it = m->begin(); m->end() != it; it++ )
    {
        if(!( this->findElement( (string *)&(it->first) ) ) )
        {
            // cppcheck-suppress cstyleCast
            if( ( n = (CppON *) it->second ) )
            {
                switch ( n->type() )
                {
                    case INTEGER_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( it->first, new COInteger( *((COInteger *) it->second ) ) );
                        break;
                    case DOUBLE_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( it->first, new CODouble( *((CODouble *) it->second ) ) );
                        break;
                    case STRING_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( it->first, new COString( *((COString *) it->second ) ) );
                        break;
                    case BOOLEAN_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( it->first, new COBoolean( *((COBoolean *) it->second ) ) );
                        break;
                    case NULL_CPPON_OBJ_TYPE:
                        fprintf( stderr, "COMap:diff - NULL type found, appended a NULL to %s\n", it->first.c_str() );
                        rtn->append( it->first, new CONull( ) );
                        break;
                    case MAP_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( it->first, new COMap( *(( COMap *) it->second ) ) );
                        break;
                    case ARRAY_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        rtn->append( it->first, new COArray( *(( COArray *) it->second ) ) );
                        break;
                    default:
                        break;
                }
            } else {
                fprintf( stderr, "appended a NULL to %s\n", it->first.c_str() );
                rtn->append( it->first, new CONull( ) );
            }
        }
    }
    if( ! rtn->size() )
    {
        delete rtn;
        rtn = NULL;
    }
    return rtn;
}

/*
 * Add an element to the map
 * But first check to see if the key is a path,
 * If not then
 *    just add it.
 * else
 *    check to see if it references an existing object in the map
 *    if not then
 *       append a map by that name
 *
 *    check to see if the object is a map
 *    if so then
 *       append the object to it with the remainder of the key
 *    if not is it an array?
 *       if so then we will append it without the key
 *    otherwise
 *       we can't do anything because we can't append an object to something that isn't a map or and array
 *
 *
 */

int COMap::append( std::string key, CppON *n )
{
    size_t pos = key.find( '/' );
    int rtn = 0;

    if( string::npos == pos )
    {
        std::map <std::string, CppON *> *m = ( std::map <std::string, CppON*> *) data;
        std::map <std::string, CppON *>::iterator it = m->find( key );									// If there is already an object by this name delete it and and the new one.
        if( m->end() != it )
        {
        	delete it->second;
        	m->erase( it );
        	std::vector< std::string>::iterator its = std::find( order.begin(), order.end(), key );
        	if( order.end() != its )
        	{
        		order.erase( its );
        	}
        }
        m->insert( m->end(), std::pair < std::string, CppON* >( key, n ) );
        order.push_back(std::string( key ) );

    } else {
        string s = key.substr( 0, pos );
        key = key.substr( pos + 1 );
        COMap *mp = (COMap *) findElement( s );
        if(! mp )
        {
            append( s, mp = new COMap() );
        }
        if( CppON::isMap( mp ) )
        {
            rtn = mp->append( key, n );
        } else if( CppON::isArray( mp ) ) {
            ((COArray *)mp )->append( n );
        } else {
             rtn = -1;
        }
    }
    return rtn;
};

// cppcheck-suppress unusedFunction
std::vector<CppON *> *COMap::getValues()
{
    std::vector<CppON *> *rtn = new std::vector<CppON *>;
    // cppcheck-suppress postfixOperator
    for(std::map< std::string, CppON *>::iterator it = (( std::map<std::string, CppON *> *) data )->begin();(( std::map<std::string, CppON *> *) data )->end() != it; it++)
    {
        rtn->push_back( it->second );
    }
    return rtn;
};

// cppcheck-suppress unusedFunction
CppON *COMap::extract( const char *name )
{
    CppON *rtn = NULL;
    std::map< std::string, CppON *>::iterator it = (( std::map<std::string, CppON *> *) data )->find( name );
    if( it != ((std::map< std::string, CppON *> *) data )->end() )
    {
        rtn = it->second;

        for( std::vector<std::string>::iterator iter = order.begin(); order.end() != iter; ++iter )
        {
            if( ! iter->compare( it->first ) )
            {
                order.erase( iter );
                break;
            }
        }

        (( std::map<std::string, CppON *> *) data )->erase( it );

    }
    return rtn;
}

COMap *COMap::operator=( COMap &val )
{
    map< string, CppON*> *ptr;
    if( data )
    {
        map <string, CppON*> *m = ( map <string, CppON *> * ) data;
        map<string, CppON *>::iterator it;
        // cppcheck-suppress postfixOperator
        for( it = m->begin(); m->end() != it; it++ )
        {
            delete( it->second);
        }
        m->clear();
    } else {
        data = new map<string, CppON*>();
    }
    order.clear();
    siz = val.size();

    map<string, CppON*> *th = (map<string, CppON*> *) data;

    if( ( ptr = val.value() ) )
    {
        // cppcheck-suppress postfixOperator
        for( std::map< string, CppON* >::iterator it = ptr->begin(); it != ptr->end(); it++ )
        {
            switch( it->second->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new COInteger( *((COInteger *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new CODouble( *( (CODouble *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new COString( *( (COString *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new CONull( *( (CONull *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new COBoolean( *( (COBoolean *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new COMap( *( (COMap *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    th->insert( th->end(), pair< string, CppON* >( it->first, new COArray( *( (COArray *)it->second ) ) ) );
                    order.push_back( string( it->first ) );
                    break;

                default:
                    break;
            }
        }
    }
    return this;
}

bool COMap::operator == ( COMap &val )
{
    map<string, CppON*> *th = (map<string, CppON*> *) data;

    // cppcheck-suppress postfixOperator
    for( std::map< string, CppON* >::iterator itr = th->begin(); itr != th->end(); itr++ )
    {
        CppON               *t;
        if( !(t = val.findElement( itr->first.c_str() ) ) )
        {
            return false;
        }
        if( !isObj( t ) || !isObj( itr->second ) )
        {
        	return false;
        }
        if( t->type() != (itr->second)->type() )
        {
        	return false;
        }
        if( *t != *itr->second )
        {
            return false;
        }
    }
    return true;
}

/****************************************************************************************/
/*																						*/
/*										COArray											*/
/*																						*/
/****************************************************************************************/

COArray::COArray( COArray *at ) : CppON( ARRAY_CPPON_OBJ_TYPE )
{
    siz = at->size();
    data = new vector<CppON *>();
    for( int i = 0; at->size() > i; i++ )
    {
        CppON *jt = at->at( i );
        switch( jt->type() )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COInteger( *((COInteger *)jt ) ) );
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new CODouble( *( (CODouble *)jt ) ) );
                break;
            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COString( *( (COString *)jt ) ) );
                break;
            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new CONull( *( (CONull *)jt ) ) );
                break;
            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COBoolean( *( (COBoolean *)jt ) ) );
                break;
            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COMap( *( (COMap *)jt ) ) );
                break;
            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COArray( *( (COArray *)jt ) ) );
                break;
            default:
                break;
        }
    }
}

COArray::COArray( COArray & at ) : CppON( ARRAY_CPPON_OBJ_TYPE )
{
    siz = at.size();
    data = new vector<CppON *>();
    for( int i = 0; at.size() > i; i++ )
    {
        CppON *jt = at.at( i );
        switch( jt->type() )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COInteger( *((COInteger *)jt ) ) );
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new CODouble( *( (CODouble *)jt ) ) );
                break;
            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COString( *( (COString *)jt ) ) );
                break;
            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new CONull( *( (CONull *)jt ) ) );
                break;
            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COBoolean( *( (COBoolean *)jt ) ) );
                break;
            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COMap( *( (COMap *)jt ) ) );
                break;
            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COArray( *( (COArray *)jt ) ) );
                break;
            default:
                break;
        }
    }
}
void COArray::parseData( const char *str )
{
    if( str )
    {
        json_t        *root = NULL;
        json_error_t  error;
        char          *np;
        int           cnt = strtol( str, &np, 0 );      // Check to see if it is a tnetstring
        string        tabs("");

        if( np )
        {
            np++;
            if( cnt && ',' == np[ cnt ] )
            {
                char *tmp = strdup( np );          // make a copy so it can be modified
                tmp[ cnt ] = '\0';              // remove the ',' at  the end of it
                root = json_loads( tmp, 0, &error );    // load it as JSON
                free( tmp );
            } else if ( cnt ) {
                root = json_loads( np, 0, &error );      // load it as JSON
            }
        }

        if( !root )                      // I guess it wasn't tnetstring so load it as JSON
        {
            if( !(str && str[ 0 ] ) )
            {
                fprintf( stderr, "%s[%d] Error: Attempt to parse zero length JSON string\n",__FILE__, __LINE__ );
            }
            root = json_loads( str, 0, &error );
        }
        if( ! root  )
        {
            fprintf( stderr, "Error: on line %d %s\n", error.line, error.text );
            fprintf( stderr, "%s\n",str );
        }
        if( json_is_array( root ) )
        {
            for( unsigned int i = 0; json_array_size( root ) > i; i++ )
            {
                this->append( parseJson( json_array_get( root, i ), tabs ) );
            }
        }
        if( root )
        {
            json_decref( root );
        }
    }
}

COArray::COArray( const char *path, const char *file ): CppON( ARRAY_CPPON_OBJ_TYPE )
{
	struct stat     _stat;
	std::string p( path );
	FILE    *fp;

    data = new vector<CppON *>();
    siz = 0;

	if( '/' != p.back() )
	{
		p += '/';
	}
	p.append( file );

	if( ! stat( p.c_str(), &_stat ) && ! ( _stat.st_mode & DIRECTORY_BIT ) && (fp = fopen( p.c_str(), "r" ) ) )
	{
	    int     sz = 1024;
	    char    *buf,*bSave;
	    int     rd = 0;
	    int     c;

	    if( !( buf = (char *) malloc( sz ) ) )
	    {
	        fclose( fp );
	        fprintf( stderr, "%s[%.4u]: Failed to allocate memory",__FILE__, __LINE__ );
	    }
	    while( EOF != ( c = fgetc( fp ) ) )
	    {
	        buf[ rd++ ] = ( char ) c;
	        if( rd == sz )
	        {
	            if( !( buf = (char *) realloc( (void *) (bSave = buf ), sz += 512 ) ) )
	            {
	    	        fprintf( stderr, "%s[%.4u]: Failed to reallocate memory: %d bytes!",__FILE__, __LINE__, sz );
	                free( bSave );
	            }
	        }
	    }
	    buf[ rd ] = '\0';
	    fclose( fp );

	    parseData( buf );

	    free(buf );
	} else {
        fprintf( stderr, "%s[%.4u]: Failed to open JSON FILE \"%s\"",__FILE__, __LINE__, p.c_str() );
	}
}

COArray::COArray( const char *str ): CppON( ARRAY_CPPON_OBJ_TYPE )
{
    data = new vector<CppON *>();
    siz = 0;
    parseData( str );
}

void COArray::clear( )
{
    vector <CppON *> *v = ( vector<CppON *> * ) data;
    for(unsigned int i = 0; v->size() > i; i++ )
    {
        delete( v->at( i ) );
    }
}

CppON *COArray::remove( size_t idx )
{
    vector< CppON *>  *v = ( vector<CppON *> *) data;
    CppON *rtn = NULL;
    if( v->size() > idx )
    {
        rtn = v->at( idx );
        v->erase( v->begin() + idx );
    }
    return rtn;
}

string *COArray::toCompactJsonString( )
{
     std::string *rtn = new string( "[" );

    if( data )
    {
        vector <CppON *> *v = ( vector <CppON *> * ) data;
        unsigned       int i;
        bool         first = true;

        for( i = 0; v->size() > i; i++ )
        {
            if( first )
            {
                first = false;
            } else {
                rtn->append( "," );
            }

            CppON       *n = v->at( i );
            std::string *sptr = NULL;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COInteger *) n )->toJsonString();
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (CODouble *) n )->toJsonString();
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COString *) n )->toJsonString();
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COBoolean *) n )->toJsonString();
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COMap *) n )->toCompactJsonString();
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COArray *) n )->toCompactJsonString( );
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    fprintf( stderr, "COArray: toJsonString -> Dropping NULL\n" );
                    break;
                default:
                    break;
            }
            if( sptr )
            {
                *rtn += *sptr;
                delete sptr;
            }
        }
    }
    *rtn += ']';
    return rtn;
}

string *COArray::toJsonString( std::string &indent )
{
    std::string *rtn = new string( indent.c_str() );
    rtn->append( "[\n" );

    if( data )
    {
        vector <CppON *> *v = ( vector <CppON *> * ) data;
        unsigned       int i;
        bool         first = true;
        std::string                       newIndent = indent;

        newIndent.append( "  " );

        for( i = 0; v->size() > i; i++ )
        {
            if( first )
            {
                first = false;
            } else {
                rtn->append( ",\n");
            }
            rtn->append( newIndent.c_str() );

            CppON       *n = v->at( i );
            std::string *sptr = NULL;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COInteger *) n )->toJsonString();
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (CODouble *) n )->toJsonString();
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COString *) n )->toJsonString();
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COBoolean *) n )->toJsonString();
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COMap *) n )->toJsonString( newIndent );
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COArray *) n )->toJsonString( newIndent );
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    fprintf( stderr, "COArray: toJsonString -> Dropping NULL\n" );
                    break;
                default:
                    break;
            }
            if( sptr )
            {
                *rtn += *sptr;
                delete sptr;
            }
        }
    }
    *rtn += '\n';
    rtn->append( indent.c_str() );
    *rtn += ']';
    return rtn;
}

string *COArray::toNetString()
{
    if( data )
    {
        vector <CppON *> *v = ( vector <CppON *> * ) data;
        unsigned       int i;
        std::string    rt;

        for( i = 0; v->size() > i; i++ )
        {
            CppON * n = v->at( i );
            std::string *sptr = NULL;
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COInteger *) n )->toNetString();
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (CODouble *) n )->toNetString();
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COString *) n )->toNetString();
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( ( COBoolean *) n )->toNetString();
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COMap *) n )->toNetString();
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    sptr = ( (COArray *) n )->toNetString();
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    fprintf( stderr, "COArray: toNetString -> Dropping NULL\n" );
                    break;
                default:
                    break;
            }
            if( sptr )
            {
                rt += *sptr;
                delete sptr;
            }
        }
        return CppON::toNetString( rt.c_str(), ']' );
    }

  return NULL;
}

const char  *COArray::c_str( std::string &idnt )
{
    str = "[";
    if( data )
    {
        vector <CppON *> *v = ( vector <CppON *> * )  data;
        unsigned                                          i;
        const char                                        *comma = "\n";
        std::string                                       indent = idnt;

        for( i = 0; v->size() > i; i++ )
        {
            CppON *n = v->at( i );
            str.append( comma );
            comma = ",\n";
            str.append( indent );
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (COInteger *) n )->c_str();
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (CODouble *) n )->c_str();
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    str += '"';
                    // cppcheck-suppress cstyleCast
                    str += ( ( COString *) n )->c_str();
                    str += '"';
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( ( COBoolean *) n )->c_str();
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( ( CONull *) n )->c_str();
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (COMap *) n )->c_str( indent );
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    str += ( (COArray *) n )->c_str( indent );
                    break;
                default:
                    break;
            }
        }
        str += '\n';
    }
    str += ']';
    return str.c_str();
}

void COArray::dump( string &indent, FILE *fp )
{
    if( data )
    {
        vector <CppON *> *v = ( vector <CppON *> * ) data;
        unsigned                                         i;
        string                                           newIndent = indent;
        bool                                             first = true;

        newIndent += "\t";
        fprintf( fp, "%s[",indent.c_str() );

        for( i = 0; v->size() > i; i++ )
        {
            CppON *n = v->at( i );
            if( first )
            {
                fprintf( fp,"\n%s", newIndent.c_str() );
                first = false;
            } else {
                fprintf( fp, ",\n%s", newIndent.c_str() );
            }
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (COInteger *) n )->dump( fp );
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (CODouble *) n )->dump( fp );
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (COString *) n )->dump( fp );
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( COBoolean *) n )->dump( fp );
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    fprintf( fp, "\n");
                    // cppcheck-suppress cstyleCast
                    ( (COMap *) n )->dump( newIndent, fp );
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    fprintf( fp, "\n");
                    // cppcheck-suppress cstyleCast
                    ( (COArray *) n )->dump( newIndent, fp );
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    fprintf( stderr, "COArray: dump -> Dropping NULL\n" );
                    break;
                default:
                    break;
            }
        }
        fprintf( fp, "\n%s]",indent.c_str() );
    } else {
        fprintf( fp, "NULL" );
    }
}
void COArray::cdump( FILE *fp )
{
    if( data )
    {
        vector <CppON *> *v = ( vector <CppON *> * ) data;
        unsigned                                         i;
        bool                                             first = true;

        fprintf( fp, "[");

        for( i = 0; v->size() > i; i++ )
        {
            CppON *n = v->at( i );
            if( first )
            {
                first = false;
            } else {
                fprintf( fp, "," );
            }
            switch ( n->type() )
            {
                case INTEGER_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (COInteger *) n )->cdump( fp );
                    break;
                case DOUBLE_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (CODouble *) n )->cdump( fp );
                    break;
                case STRING_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( (COString *) n )->cdump( fp );
                    break;
                case BOOLEAN_CPPON_OBJ_TYPE:
                    // cppcheck-suppress cstyleCast
                    ( ( COBoolean *) n )->cdump( fp );
                    break;
                case MAP_CPPON_OBJ_TYPE:
                    fprintf( fp, "\"\n\"");
                    // cppcheck-suppress cstyleCast
                    ( (COMap *) n )->cdump( fp );
                    break;
                case ARRAY_CPPON_OBJ_TYPE:
                    fprintf( fp, "\"\n\"");
                    // cppcheck-suppress cstyleCast
                    ( (COArray *) n )->cdump( fp );
                    break;
                case NULL_CPPON_OBJ_TYPE:
                    fprintf( stderr, "COArray: cdump -> Dropping NULL\n" );
                    break;
                default:
                    break;
            }
        }
        fprintf( fp, "]" );
    } else {
        fprintf( fp, "[]" );
    }
}

COArray  *COArray::diff( COArray &newObj, const char *name )
{
    COArray                           *rtn   = new COArray();
    COMap                             *nv;
    COString                          *vS;
    COArray                           *na;
    vector< CppON *>::iterator      it;
    vector< CppON *>::iterator      nt;
    vector< CppON *>                *v     = ( vector <CppON *> * ) data;
    vector< CppON *>                *u     = ( vector <CppON *> * ) newObj.data;
    it = v->begin();
    // cppcheck-suppress postfixOperator
    for( nt = u->begin(); u->end() != nt; nt++ )
    {
        CppON  *n;
        CppON  *obj = NULL;
        COString *uS;
        // cppcheck-suppress cstyleCast
        if( CppON::isMap( obj = (CppON *) *nt ) && name && CppON::isString( uS = (COString *)( ( COMap * ) obj)->findElement( name ) ) )   // If array of maps look for name
        {
            // cppcheck-suppress postfixOperator
            for( it = v->begin(); v->end() != it; it++ )
            {
                // cppcheck-suppress cstyleCast
                if( CppON::isMap( n = (CppON *) *it ) && CppON::isString( vS = (COString *) ( ( COMap * ) n )->findElement( name ) ) && !strcmp( uS->c_str(), vS->c_str( ) ) )
                {
                    // cppcheck-suppress cstyleCast
                    if( (nv = ((COMap *) n )->diff( *((COMap *) obj ), name ) ) )
                    {
                        delete( nv );      // send whole map
                        // cppcheck-suppress cstyleCast
                        rtn->append( new COMap( *( ( COMap * ) obj ) ) );
                        break;
                    }
                }
            }
        } else if( it != v->end() ) {
            obj = ( CppON * ) *nt;
            n = ( CppON * ) *it;
            if( obj->type() == n->type() )
            {
                switch ( obj->type() )
                {
                    case INTEGER_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((COInteger *) n ) != *((COInteger *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( new COInteger( *((COInteger *) obj) ) );
                        }
                        break;

                    case DOUBLE_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((CODouble *) n ) != *((CODouble *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( new CODouble( *((CODouble *) obj) ) );
                        }
                        break;

                    case STRING_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((COString *) n ) != *((COString *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( new COString( *((COString *) obj) ) );
                        }
                        break;

                    case BOOLEAN_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( *((COBoolean *) n ) != *((COBoolean *) obj ) )
                        {
                            // cppcheck-suppress cstyleCast
                            rtn->append( new COBoolean( *((COBoolean *) obj) ) );
                        }
                        break;

                    case NULL_CPPON_OBJ_TYPE:
                        fprintf( stderr, "COArray: diff - Dropping NULL\n" );
                        break;
                    case MAP_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( (nv = ((COMap *) n )->diff( *((COMap *) obj ) ) ) )
                        {
                            delete (nv);                                                  // send whole map
                            // cppcheck-suppress cstyleCast
                            rtn->append( new COMap(  *((COMap *) obj ) ) );
                        }
                        break;
                    case ARRAY_CPPON_OBJ_TYPE:
                        // cppcheck-suppress cstyleCast
                        if( (na = ( (COArray *) n )->diff( *( ( COArray *) obj ) ) ) )
                        {
                            delete (na);                                                  // send whole map
                            // cppcheck-suppress cstyleCast
                            rtn->append( new COArray(  *((COArray *) obj ) ) );
                        }
                        break;
                    default:
                        break;
                }
            }
            // cppcheck-suppress postfixOperator
            it++;
        }
    }
    if(!rtn->size() )
    {
        delete rtn;
        rtn = NULL;
    }
    return rtn;
}

COArray *COArray::operator=( COArray &val )
{
    if( data )
    {
        vector <CppON *> *v = ( vector<CppON *> * ) data;
        for(unsigned int i = 0; v->size() > i; i++ )
        {
            delete( v->at( i ) );
        }
        v->clear();
    } else {
        data = new vector<CppON *>();
    }
    siz = val.size();

    for( int i = 0; val.size() > i; i++ )
    {
        CppON *jt = val.at( i );

        switch( jt->type() )
        {
            case INTEGER_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COInteger( *((COInteger *)jt ) ) );
                break;
            case DOUBLE_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new CODouble( *( (CODouble *)jt ) ) );
                break;
            case STRING_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COString( *( (COString *)jt ) ) );
                break;
            case NULL_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new CONull( *( (CONull *)jt ) ) );
                break;
            case BOOLEAN_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COBoolean( *( (COBoolean *)jt ) ) );
                break;
            case MAP_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COMap( *( (COMap *)jt ) ) );
                break;
            case ARRAY_CPPON_OBJ_TYPE:
                // cppcheck-suppress cstyleCast
                append( new COArray( *( (COArray *)jt ) ) );
                break;
            default:
                break;
        }
    }
    return this;
}

bool COArray::operator == ( COArray &val )
{
    // cppcheck-suppress cstyleCast
    if( val.size() != ((COArray*)this)->size() )
    {
        return false;
    }
    for( int i = 0; val.size() > i; i++ )
    {
        // cppcheck-suppress cstyleCast
        if( ! ( val.at( i ) == ((COArray*)this)->at( i ) ) )
        {
            return false;
        }
    }
    return true;
}

/****************************************************************************************/
/*                                                                                      */
/*                                 COString                                             */
/*                                                                                      */
/****************************************************************************************/

COString::COString( COString *st ) : CppON( STRING_CPPON_OBJ_TYPE )
{
	data = ( st->data ) ? new std::string( ( ( std::string *) st->data )->c_str() ): NULL;
}

COString::COString( COString &st ) : CppON( STRING_CPPON_OBJ_TYPE )
{
    data = ( st.data ) ? new std::string( ( ( std::string *) st.data )->c_str() ): NULL;
}

COString::COString( std::string st ) : CppON( STRING_CPPON_OBJ_TYPE )
{
	std::string rst;

	for( unsigned int i = 0; st.length() > i; i++ )
	{
		char ch = st[ i ];
		switch( ch )
		{
        	case '"':
        		rst.append( "%22" );
        		break;
        	case '%':
        		rst.append( "%25" );
        		break;
        	case '\0':
        		rst.append( "%00" );
        		break;
        	default:
        		rst.push_back( ch );
        		break;
		}
	}
	data = new std::string( rst.c_str() );
}

COString::COString( std::string st, bool base64 ) : CppON( STRING_CPPON_OBJ_TYPE )
{
	if( ! base64 )
	{
		std::string rst;

		for( unsigned int i = 0; st.length() > i; i++ )
		{
			char ch = st[ i ];
			switch( ch )
			{
            	case '"':
            		rst.append( "%22" );
            		break;
            	case '%':
            		rst.append( "%25" );
            		break;
            	case '\0':
            		rst.append( "%00" );
            		break;
            	default:
            		rst.push_back( ch );
            		break;
			}
		}
		data = new std::string( rst.c_str() );
	} else {
		unsigned int	len;
		char 			out[ st.length() + 3 ];
		if( base64Decode( st.c_str(), st.length(), len, out ) )
		{
			std::string *s = new std::string( );
			s->reserve( len );
			for( unsigned i = 0; i < len; i++ )
			{
				s[ i ] = out[ i ];
			}
		} else {
			data = NULL;
		}
	}
};

COString::COString( const char *st, bool base64) : CppON( STRING_CPPON_OBJ_TYPE )
{
	if( ! base64 )
	{
		data = new std::string( st );
	} else {
		unsigned int	len;
		unsigned int	sz = strlen( st );
		char 			out[ sz + 3 ];

		if( base64Decode( st, sz, len, out ) )
		{
			std::string *s = new std::string( );
			s->reserve( len );
			data = s;
			for( unsigned i = 0; i < len; i++ )
			{
				s->push_back( out[ i ] );
			}
		} else {
			data = NULL;
		}
	}
}

COString::COString( uint64_t val, bool hex ) : CppON( STRING_CPPON_OBJ_TYPE )
{
	char buf[ 32 ];
#if SIXTY_FOUR_BIT
	if( hex )
	{
		snprintf( buf, 31, "0x%.16lX", val );
	} else {
		snprintf( buf, 31, "%lu", val );
	}
#else
	if( hex )
	{
		snprintf( buf, 31, "0x%.16llX", val );
	} else {
		snprintf( buf, 31, "%llu", val );
	}

#endif
	data = new std::string( buf );
}
COString::COString( uint32_t val, bool hex ) : CppON( STRING_CPPON_OBJ_TYPE )
{
	char buf[ 24 ];
	if( hex )
	{
		snprintf( buf, 23, "0x%.8X", val );
	} else {
		snprintf( buf, 23, "%u", val );
	}
	data = new std::string( buf );
}
COString *COString::operator = ( uint64_t val )
{
	char buf[ 32 ];
#if SIXTY_FOUR_BIT
	if( data && '0' == ((std::string *) data)->at( 0 )  )
	{
		snprintf( buf, 31, "0x%.16lX", val );
	} else {
		snprintf( buf, 31, "%lu", val );
	}
#else
	if( data && '0' == ((std::string *) data)->at( 0 )  )
	{
		snprintf( buf, 31, "0x%.16llX", val );
	} else {
		snprintf( buf, 31, "%llu", val );
	}
#endif
	if( data )
	{
		*( ( std::string *) data) = buf;
	} else {
		data = new std::string( buf );
	}
	return this;
}
COString *COString::operator = ( uint32_t val )
{
	char buf[ 24 ];
	if( data && '0' == ((std::string *) data)->at( 0 ) )
	{
		snprintf( buf, 23, "0x%.16X", val );
	} else {
		snprintf( buf, 23, "%u", val );
	}
	if( data )
	{
		*( ( std::string *) data) = buf;
	} else {
		data = new std::string( buf );
	}
	return this;
}
COString *COString::operator = ( int val )
{
	char buf[ 24 ];
	snprintf( buf, 23, "%d", val );

	if( data )
	{
		*( ( std::string *) data) = buf;
	} else {
		data = new std::string( buf );
	}
	return this;
}
// cppcheck-suppress unusedFunction
std::string *COString::toString()
{
    char        *cPtr = (char *) data;
    char        ch;
    std::string *rtn = new string();

    while( 0 != (ch = *cPtr++ ) )
    {
        if( '%' == ch )
        {
            ch = ( ( *cPtr++ - '0' ) << 4 );
            ch += (*cPtr++ - '0' );
        }
        rtn->push_back( ch );
    }
    rtn->push_back( '\0' );
    return rtn;
};

string *COString::toNetString()
{
    return CppON::toNetString( ( ( std::string *) data )->c_str(), ',' );
}

static unsigned char dtab[] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x0A, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3E, 0x80, 0x80, 0x80, 0x3F,
                0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
                0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
                0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00};

static unsigned char etable[] = {0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,
            0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x61,0x62,0x63,0x64,0x65,0x66,
            0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
            0x77,0x78,0x79,0x7A,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2B,0x2F};


/*
 * Decode Base64 encoded strings
 * Some files insert newlines in the strings to brake them up.  Allow those
 */
char *COString::base64Decode( const char *tmp, unsigned int sz, unsigned int &len, char	*out )
{
	int i;
	int ch;
	unsigned char a[ 4 ],b[ 4 ];

	len = 0;
	if( ! tmp || 1 > sz )
    {
		return NULL;
    }

	if( ! out )
	{
		out = new char[ sz + 3 ];
	}

	while( *tmp != 0 )
	{
		if( 0x0A == *tmp )
		{
			tmp++;
		} else {
			for( i = 0; 4 > i ;  )
			{
				ch = *tmp++;
				if( ch == 0 || 0x80 == dtab[ ch ] )          // Got an illegal character so die.
				{
					fprintf( stderr, "Character = 0x%.2X => dtab: 0x%.2X\n", ch, dtab[ ch ] );
					return NULL;
				} else if( 0x0A != ch ) {
					a[ i ] = (unsigned char) ch;
					b[ i++ ] = (unsigned char) dtab[ ch ];
				}
			}
			out[ len++ ] = ( b[ 0 ] << 2 ) | ( b[ 1 ] >> 4 );
			if(a[ 2 ] != '=')
			{
				out[ len++ ] = ( b[ 1 ] << 4 ) | ( b[ 2 ] >> 2 );
				if(a[ 3 ] != '=')
				{
					out[ len++ ] = ( b[ 2 ] << 6 ) | b[ 3 ];
				}
			}
		}
	}
	out[ len ] = 0;
	return out;
}

std::string *COString::toBase64JsonString( const char *cPtr, unsigned int len )
{
	unsigned char o[ ( ( len * 4 ) / 3 ) + 4 ];  									// Make sure it is big enough to hold the output data
	unsigned char ig[ 3 ];
	unsigned char c;
	int n;
	unsigned char *ot = o;
	bool done=false;

	while( !done )
	{
		ig[ 0 ] = ig[ 1 ] = ig[ 2 ] = 0;
	    for( n = 0; 3 > n; n++ )
	    {
	    	if((c = (unsigned char)*cPtr++) == 0)  // end of input string so we are done....
	    	{
	    		done = true;
	    		break;
	    	}
	    	ig[ n ] = c;
	    }
	    if( 0 < n )
	    {
	    	*ot++ = etable[ ig[ 0 ] >> 2 ];
	    	*ot++ = etable[ ( ( ig[ 0 ] & 3) << 4 ) | ig[ 1 ] >> 4 ];
	    	*ot++ = etable[ ( ( ig[ 1 ] &0x0f ) << 2 ) | ( ig[ 2 ] >> 6 ) ];
	    	*ot++ = etable[ ig[ 2 ] & 0x3f ];

	    	if( 3 > n )                 // If we kicked out before getting all 3 input bytes then replace with '='
	    	{
	    		ot[ -1 ] = '=';
	    		if(2 > n )
	    		{
	    			ot[ -2 ] = '=';
	    		}
	    	}
	    }
	}
	*ot = '\0';
	return new std::string( (char *) o );
}

string *COString::toJsonString()
{
    if( ! data )
    {
        return NULL;
    }
    unsigned int len = ( ( std::string *) data)->length();
    const char *cPtr =  ( ( std::string *) data)->c_str();
    string *rtn = new string( "\"" );
    for(unsigned int i = 0; len > i; i++)
    {
        char ch = cPtr[ i ];
        switch( ch )
        {
            case '"':
                rtn->append( "%22");
                break;
            case '{':
                rtn->append( "%7B" );
                break;
            case '}':
                rtn->append( "%7D" );
                break;
            case '<':
                rtn->append( "%3C" );
                break;
            case '>':
                rtn->append( "%3E" );
                break;
            case '\\':
                rtn->append( "%5C" );
                break;
            case '\'':
                rtn->append( "%60" );
                break;
            case '^':
                rtn->append( "%5E" );
                break;
            case '&':
                rtn->append( "%26" );
                break;
            case '\r':
                rtn->append( "%0D" );
                break;
            case '\n':
            case '\a':
                rtn->append( "%0A" );
                break;
            case '\t':
                rtn->push_back( ' ' );
                break;
            default:
                rtn->push_back( ch );
                break;
        }
    }
    rtn->push_back( '"');
    return rtn;
}

void COString::dump( FILE *fp)
{
    fprintf( fp, "\"%s\"", ( data ) ? ((std::string *) data)->c_str() : "\"\"" );
}
void COString::cdump( FILE *fp )
{
    fprintf( fp, "\\\"%s\\\"", ( data ) ? ((std::string *) data)->c_str() : "\"\"" );
}

/****************************************************************************************/
/*                                                                                      */
/*                                        CODouble                                      */
/*                                                                                      */
/****************************************************************************************/

CODouble::CODouble( CODouble *dt ) : CppON( DOUBLE_CPPON_OBJ_TYPE )
{
    if( ! data )
    {
        data = new ( double );
    }
    *((double *) data) = dt->doubleValue();
    precision = dt->Precision();
}

CODouble::CODouble( CODouble &dt ) : CppON( DOUBLE_CPPON_OBJ_TYPE )
{
    if( ! data )
    {
        data = new ( double );
    }
    *((double *) data) = dt.doubleValue();
}

string *CODouble::toNetString()
{
    char buf[ 48 ];
    buf[ 47 ] = buf[ 0 ] = '\0';
    if( data )
    {
        snprintf( buf, 47, "%.10lf", *(( double *) data ));
    }
    return CppON::toNetString( buf, '^' );
};

string *CODouble::toJsonString()
{
    char buf[ 128 ];
    buf[127] = buf[ 0 ] = '\0';
    if( data )
    {
        if( 0 > precision || 16 < precision )
        {
        	snprintf( buf, 127, "%.10lf", *(double *)data );
        } else{
            char prebuf[ 8 ];
            prebuf[ 7 ] = buf[ 23 ] = '\0';
            snprintf( prebuf, 7, "%c.%dlf", '%', (int)precision );
            snprintf( buf, 23, prebuf, *(double *) data );
        }
    }
    return new string( buf );
}

const char *CODouble::c_str()
{
    char buf[ 32 ];
    buf[ 31 ] = '\0';
    if( data )
    {
        if( 0 > precision || 16 < precision )
        {
            snprintf( buf, 31, "%.10lf", *(double *) data );
        } else{
            char prebuf[ 8 ];
            prebuf[ 7 ] = buf[ 23 ] = '\0';
            snprintf( prebuf, 7, "%c.%dlf", '%', (int)precision );
            snprintf( buf, 23, prebuf, *(double *) data );
        }
    } else {
        strcpy( buf, "NULL" );
    }
    str = buf;
    return str.c_str();
}

void CODouble::dump( FILE *fp)
{
    if( data )
    {
        fprintf( fp, "%.10lf", *(double *) data );
    } else {
        fprintf( fp, "NULL" );
    }
}

void CODouble::cdump( FILE *fp )
{
    if( data )
    {
        fprintf( fp, "%.16lf", *(double *) data );
    } else {
        fprintf( fp, "0.0" );
    }
}

double CODouble::operator = (const double& val)
{
    if( !data )
    {
        data = new double;
        *((double*) (*this).data) = val;
    } else {
        if( 0 > precision  || 16 < precision )
        {
            *((double*) (*this).data) = val;
        } else {
            double pow_10 = pow( 10.0, precision);
            double n = pow_10 * *((double*) (*this).data);
            double d = pow_10 * val;
            double t = n - d;
            if( 0.75 < t || -0.75 > t )
            {
                *((double*) (*this).data) = round( d )/ pow_10;
            }
        }
    }
    return *((double*) (*this).data);

}

CODouble *CODouble::operator = ( CODouble &val)
{
    if( ! data )
    {
        data = new double;
        precision = val.precision;
        *((double*)data ) = val.doubleValue();
    } else {
        if( 0 <= precision && 16 >= precision )
        {
            double pow_10 = pow( 10.0, precision);
            double n = pow_10 * *((double*) (*this).data);
            double d = pow_10 * val.doubleValue();
            double t = n - d;
            if( 0.75 < t || -0.75 > t )
            {
                *((double*)data ) = round( d )/ pow_10;
            }
        } else {
            *((double*)data ) = val.doubleValue();
        }
    }
    return this;
};

/****************************************************************************************/
/*																						*/
/*										COInteger										*/
/*																						*/
/****************************************************************************************/

string *COInteger::toJsonString()
{
    char buf[ 32 ];
    buf[ 31 ] = buf[ 0 ] = '\0';
    if( data )
    {
        switch ( siz )
        {
            case sizeof( char ):
                snprintf( buf, 31,"%c", *( char *) data );
                break;
            case sizeof( short ):
                snprintf( buf, 31, "%d", ( int ) *(( short *) data ));
                break;
            case sizeof( int ):
                snprintf( buf, 31, "%d", *(( int *) data ));
                break;
            case sizeof( long long):
#if SIXTY_FOUR_BIT
                snprintf( buf, 31, "%ld", *(( uint64_t *) data ));
#else
                snprintf( buf, 31, "%lld", *(( uint64_t *) data ));
#endif
                break;
        }
    }
    return new string( buf );
}

COInteger::COInteger( COInteger *it ) : CppON( INTEGER_CPPON_OBJ_TYPE )
{
    switch( siz = it->size() )
    {
        case sizeof( char ):
            data = new ( char );
            *((char*) data) = it->charValue();
            break;
        case sizeof( short ):
            data = new ( short );
            *((short*) data) = it->shortValue();
            break;
        case sizeof( int ):
            data = new ( int );
            *((int*) data) = it->intValue();
            break;
        default:
            data = new ( long long );
            *((long long*) data) = it->longValue();
            break;
    }
}

COInteger::COInteger( COInteger &it ) : CppON( INTEGER_CPPON_OBJ_TYPE )
{
    switch( siz = it.size() )
    {
        case sizeof( char ):
            data = new ( char );
            *((char*) data) = it.charValue();
            break;
        case sizeof( short ):
            data = new ( short );
            *((short*) data) = it.shortValue();
            break;
        case sizeof( int ):
            data = new ( int );
            *((int*) data) = it.intValue();
            break;
        default:
            data = new ( long long );
            *((long long*) data) = it.longValue();
            break;
    }
}

long long COInteger::longValue()
{
    long long rtn = 0;
    if( data )
    {
        switch ( siz )
        {
            case sizeof( char ):
                rtn = (long long) *( (char *) data );
                break;
            case sizeof( short ):
                rtn = (long long) *( ( short *) data );
                break;
            case sizeof( int ):
                rtn = (long long) *( (int *) data );
                break;
            case sizeof( long long ):
                rtn = *( ( long long *) data );
                break;
        }
    }
    return rtn;
}

string *COInteger::toNetString()
{
    char buf[ 32 ];
    buf[ 31 ] = buf[ 0 ] = '\0';
    if( data )
    {
        switch ( siz )
        {
            case 1:
                snprintf( buf, 31, "%c", *( char *) data );
                break;
            case 2:
                snprintf( buf, 31, "%d", ( int ) *(( short *) data ));
                break;
            case 4:
                snprintf( buf, 31, "%d", *(( int *) data ));
                break;
            case 8:
#if SIXTY_FOUR_BIT
                snprintf( buf, 31, "%ld", *(( uint64_t *) data ));
#else
                snprintf( buf, 31, "%lld", *(( uint64_t *) data ));
#endif
                break;
        }
    }
    return CppON::toNetString( buf, '#' );
}

const char *COInteger::c_str()
{
    char buf[ 32 ];
    buf[ 31 ] = '\0';
    if( data )
    {
        switch ( siz )
        {
            case sizeof( char ):
                snprintf( buf, 31, "%c", *(char *) data );
                break;
            case sizeof( short ):
                snprintf( buf, 31, "%d", ( int ) *(short *) data );
                break;
            case sizeof( int ):
                snprintf( buf, 31, "%d", *(int *) data );
                break;
            default:
#if SIXTY_FOUR_BIT
                snprintf( buf, 31, "%ld", *(uint64_t *) data );
#else
                snprintf( buf, 31, "%lld", *(uint64_t *) data );
#endif
                break;
        }
    }
    str = buf;
    return str.c_str();
}

void COInteger::dump( FILE *fp)
{
    if( data )
    {
        switch ( siz )
        {
            case sizeof( char ):
                fprintf( fp, "%c", *(char *) data );
                break;
            case sizeof( short ):
                fprintf( fp, "%d", ( int ) *(short *) data );
                break;
            case sizeof( int ):
                fprintf( fp, "%d", *(int *) data );
                break;
            default:
#if SIXTY_FOUR_BIT
                fprintf( fp, "%ld", *(uint64_t *) data );
#else
                fprintf( fp, "%lld", *(uint64_t *) data );
#endif
                break;
            }
    } else {
        fprintf( fp, "NULL" );
    }
}

void COInteger::cdump( FILE *fp )
{
    if( data )
    {
        switch ( siz )
        {
            case sizeof( char ):
                fprintf( fp, "%c", *(char *) data );
                break;
            case sizeof( short ):
                fprintf( fp, "%d", ( int ) *(short *) data );
                break;
            case sizeof( int ):
                fprintf( fp, "%d", *(int *) data );
                break;
            default:
#if SIXTY_FOUR_BIT
                fprintf( fp, "%ld", *(uint64_t *) data );
#else
                fprintf( fp, "%lld", *(uint64_t *) data );
#endif
                break;
            }
    } else {
        fprintf( fp, "0" );
    }
}


uint64_t COInteger::doOperation( uint64_t val, CppONOperator op )
{
	uint64_t rtn = 0;
	switch( siz )
	{
		case 8:
			switch( op )
			{
				case CPPON_ADD:
					*((char*) data ) += (char) val;
					rtn = (int64_t) *((char*) data );
					break;
				case CPPON_U_ADD:
					*((unsigned char*) data ) += (unsigned char) val;
					rtn = (uint64_t) *((unsigned char*) data );
					break;
				case CPPON_SUBTRACT:
					*((char*) data ) -= (char) val;
					rtn = (int64_t) *((char*) data );
					break;
				case CPPON_U_SUBTRACT:
					*((unsigned char*) data ) -= (unsigned char) val;
					rtn = (uint64_t) *((unsigned char*) data );
					break;
				case CPPON_MULTIPLY:
					*((char*) data ) *= (char) val;
					rtn = (int64_t) *((char*) data );
					break;
				case CPPON_U_MULTIPLY:
					*((unsigned char*) data ) *= (unsigned char) val;
					rtn = (uint64_t) *((unsigned char*) data );
					break;
				case CPPON_DIVIDE:
					*((char*) data ) /= (char) val;
					rtn = (int64_t) *((char*) data );
					break;
				case CPPON_U_DIVIDE:
					*((unsigned char*) data ) /= (unsigned char) val;
					rtn = (uint64_t) *((unsigned char*) data );
					break;
			}
			break;
		case 16:
			switch( op )
			{
				case CPPON_ADD:
					*((short*) data ) += (short) val;
					rtn = (int64_t) *((short*) data );
					break;
				case CPPON_U_ADD:
					*((short*) data ) += (short) val;
					rtn = (uint64_t) *((unsigned short*) data );
					break;
				case CPPON_SUBTRACT:
					*((short*) data ) -= (short) val;
					rtn = (int64_t) *((short*) data );
					break;
				case CPPON_U_SUBTRACT:
					*((short*) data ) -= (short) val;
					rtn = (uint64_t) *((unsigned short*) data );
					break;
				case CPPON_MULTIPLY:
					*((short*) data ) *= (short) val;
					rtn = (int64_t) *((short*) data );
					break;
				case CPPON_U_MULTIPLY:
					*((short*) data ) *= (short) val;
					rtn = (uint64_t) *((unsigned short*) data );
					break;
				case CPPON_DIVIDE:
					*((short*) data ) /= (short) val;
					rtn = (int64_t) *((short*) data );
					break;
				case CPPON_U_DIVIDE:
					*((short*) data ) /= (short) val;
					rtn = (uint64_t) *((unsigned short*) data );
					break;
			}
			break;
		case 32:
			switch( op )
			{
				case CPPON_ADD:
					*((int32_t *) data ) += (int32_t) val;
					rtn = (int64_t) *((int32_t*) data );
					break;
				case CPPON_U_ADD:
					*((uint32_t *) data ) += (uint32_t) val;
					rtn = (uint64_t) *((uint32_t*) data );
					break;
				case CPPON_SUBTRACT:
					*((int32_t*) data ) -= (int32_t) val;
					rtn = (int64_t) *((int32_t*) data );
					break;
				case CPPON_U_SUBTRACT:
					*((int32_t*) data ) -= (uint32_t) val;
					rtn = (uint64_t) *((uint32_t*) data );
					break;
				case CPPON_MULTIPLY:
					*((int32_t*) data ) *= (int32_t) val;
					rtn = (int64_t) *((int32_t*) data );
					break;
				case CPPON_U_MULTIPLY:
					*((uint32_t*) data ) *= (uint32_t) val;
					rtn = (uint64_t) *((uint32_t*) data );
					break;
				case CPPON_DIVIDE:
					*((int32_t*) data ) /= (int32_t) val;
					rtn = (int64_t) *((int32_t*) data );
					break;
				case CPPON_U_DIVIDE:
					*((uint32_t*) data ) /= (uint32_t) val;
					rtn = (uint64_t) *((uint32_t*) data );
					break;
			}
			break;
		case 64:
			switch( op )
			{
				case CPPON_ADD:
					*((int64_t *) data ) += (int64_t) val;
					rtn = (uint64_t) *((int64_t*) data );
					break;
				case CPPON_U_ADD:
					*((uint64_t *) data ) += (uint64_t) val;
					rtn = (uint64_t) *((uint64_t*) data );
					break;
				case CPPON_SUBTRACT:
					*((int32_t*) data ) -= (int64_t) val;
					rtn = (int64_t) *((int64_t*) data );
					break;
				case CPPON_U_SUBTRACT:
					*((uint32_t*) data ) -= (uint64_t) val;
					rtn = (uint64_t) *((uint64_t*) data );
					break;
				case CPPON_MULTIPLY:
					*((int32_t*) data ) *= (int64_t) val;
					rtn = (int64_t) *((int64_t*) data );
					break;
				case CPPON_U_MULTIPLY:
					*((uint32_t*) data ) *= (uint64_t) val;
					rtn = (uint64_t) *((uint64_t*) data );
					break;
				case CPPON_DIVIDE:
					*((int32_t*) data ) /= (int64_t) val;
					rtn = (int64_t) *((int64_t*) data );
					break;
				case CPPON_U_DIVIDE:
					*((uint32_t*) data ) /= (uint64_t) val;
					rtn = (uint64_t) *((uint64_t*) data );
					break;
			}
			break;
	}
	return rtn;
}


bool COInteger::operator == ( COInteger &newObj ) {
    /*
     * If the sizes are equal then just compare them
     */
    if( siz == newObj.siz )
    {
        return (
                ( siz == sizeof( long long) ) ? *( (long long *) newObj.data ) == *( (long long *)data ) :
                ( siz == sizeof( int ) )      ? *( (int *)       newObj.data ) == *( (int *)      data ) :
                ( siz == sizeof( short ) )    ? *( (short *)     newObj.data ) == *( (short *)    data ) :
                *( (char * )     newObj.data ) == *( (char *)     data ) );
        /*
         * Else convert them both to "long long" and compare them.
         */
    } else {
        return ( (siz == sizeof( long long ) ) ?                 *( (long long *)         data ) :
               ( (siz == sizeof( int       ) ) ? ( long long ) *( (int *)               data ) :
               ( (siz == sizeof( short     ) ) ? ( long long ) *( (short *)             data ) :
        ( long long ) *( (char *)              data ) ) ) ) ==
               ( (newObj.siz == sizeof( long long ) ) ?               *( (long long *)  newObj.data ) :
               ( (newObj.siz == sizeof( int       ) ) ? ( long long ) *( (int *)        newObj.data ) :
               ( (newObj.siz == sizeof( short     ) ) ? ( long long ) *( (short *)      newObj.data ) :
                                               ( long long ) *( (char *)       newObj.data ) ) ) );
    }
}

COInteger *COInteger::operator=(COInteger &val )
{
    if( siz != val.siz )
    {
        switch(siz)
        {
            case sizeof(int): delete (int*)data; break;
            case sizeof(long long): delete (long long*)data; break;
            case sizeof(short): delete (short*)data; break;
            case sizeof(char): delete (char*)data; break;
        }
        switch(siz=val.siz)
        {
            case sizeof(int): data=new int;break;
            case sizeof(long long): data=new (long long); break;
            case sizeof(short): data=new(short); break;
            case sizeof(char): data=new(char); break;
        }
    }
    switch(siz)
    {
        case sizeof(int): *((int*)data )=val.intValue(); break;
        case sizeof(long long): *((long long*)data )=val.longValue(); break;
        case sizeof(short): *((short*)data )=val.shortValue(); break;
        case sizeof(char): *((char*)data )=val.charValue(); break;
    }
    return this;
}

/****************************************************************************************/
/*																						*/
/*										CONull											*/
/*																						*/
/****************************************************************************************/

void CONull::dump( FILE *fp)
{
    fprintf( fp, "NULL" );
}
void CONull::cdump( FILE *fp )
{
    fprintf( fp, "null" );
}
string *CONull::toJsonString()
{
    return new string("null");
}
