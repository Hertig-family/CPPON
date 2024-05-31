         File Name: Readme.txt
        Created on: 10 Jan 2010 - 26 May 2024
            Author: Jeff Hertig
       Description: CPPON - C++ Object Notation.  In short, this is JSON for C++, made easy.  Its original intent was to make working with JSON 
                    messages in C++ easy but it became much, much more.  It actually start as a library meant to may working with XML messages easier,
                    but really found it place when JSON came on the scene. The name has been changed many times over the years but because its 
                    fundamental purpose is to do for C++ what JSON does for JavaScript its name has recently been changed to what it is now.
                   
        What it is: It is a C++ Shared Library which implements a class hierarchy representing a JSON message.  It can take a JSON message and insatiate
                    a C++ class structure representing all the elements of the message and allow them to be read/manipulated by the C++ program. The 
                    data objects and/or structure can be accessed/manipulated using “paths” in much the same way as it is in JavaScript.

                    The class hierarchy is all derived from a single base class with two container classes and five base objects representing float,
                    integer, string, boolean, and the null data types.  The two “container” classes which are implemented as a “std::map” and a 
                    “std::vector” (complete with iterators) can hold any of the object types.

                    Any valid JSON string or file can be parsed into a complete data structure with a single command without the need to even know the 
                    messages contents. The system also works with TNetStrings (and certain XML if compiled with the option to do so.)  Of course the 
                    structure can also be created problematically either a string or built up element by element in the code. (Note: When using 
                    TNetStrings or XML binary is supported but caution is advised when/if then attempting to convert it to JSON)

                    At any point a single method can be called to create a string or file from the hierarchy in either a verbose, human readable form or
                    a compact (sans white space) form.

           History: As earlier stated, This started in early 2010 as a means of working with XML encoded messages.  But as XML is very flexible it often
                    required customization depending on the message structure/content.  When JSON became popular this was no long an issue as all JSON
                    can be represented by one of seven objects. (Two container and five base objects)

                    As JSON became the de facto standard for massage protocol the library because very useful and XML was pretty much abandoned. (It is 
                    still available but requires additional dependent libraries.)

                    Soon the library because valuable in another unforeseen way.  It became a way to implement “Soft Structures” in C++:  Internal data
                    structures can now be defined by JSON configuration files and instantiated at run time.  While obvious not as “high performance” as
                    hard-coded data structures and Class structures, in the embedded equipment world where configurations often change being able to 
                    define the internal data structures from a configuration file has some real advantages.
                    
                    Soon whole systems were being designed around the concept of “soft structures” which lead to a problem.  The class structure could
                    be instantiated and in a single application but when multiple applications needed the same information they had to have their own 
                    structure and rely on complex message queues to synchronize the data.

                    This gave rise to a companion class structure the Shared C++ Object (SCppObj) which when instantiated existed in system shared 
                    memory.  This class mimics the original CPPON class but exists outside of any one application and has built in protection against 
                    data corruption from simultaneous access.  Although this class has one serious limitation:  Once created (and initialized) it’s data
                    structure is fixed.  It exists with the same data structure until it is intentionally destroyed (and all references to it are closed)
                    or a system reboot.

                    The big advantage of this class is that it allows the “soft structure” implementation to be system wide with the same flexibility of
                    being defined by a JSON file where the system state can be stored/read in real time by many different applications.

                    This leads to another issue.  When something changes in the SCppObj class the applications have no way of knowing the data has changed
                    without comparing it to another local value or checking the update time.  This can be a rather tedious process.

                    For that reason another companion class was added.  The Local C++ Object (LocalCppObj) class.  This class has one function.  It is a 
                    local image of the ScppObj class that when queried will return a list of things that changed since the last time it was queried.

                    Of course the response is in the form of a CPPON instance which can of course be either iterated through or used to produce a JSON 
                    string to be sent as a message.
           example: /*
                     * A little demonstration program to show some features of the C++ Object Notation library (CPPON).  This program takes a string as if it was received as a JSON message
                     * from an accounting department request the ours the some people worked.  It reads the hours worked per day from a JSON file stored in the local director, adds the hours
                     * for each person and returns a JSON string giving the hours.
                     *
                     * It is a simple program and meant to show a variety of features, not necessarily the best way to do the operation.
                     *
                     * The file red from the disk contains the following data: in compact JSON form:
                     *
                     * 		{"Week":"5/22/2024","hours":{"Alice":{"Monday":8.2,"Tuesday":8.1,"Wednesday":8.5,"Thursday":7.9,"Firday":8.0},"Fred":{"Monday":8.0,"Tuesday":8.3,"Wednesday":8.1, \
                     *      "Thursday":8.8,"Firday":8.0},"Sam":{"Monday":8.2,"Tuesday":8.6,"Wednesday":8.0,"Thursday":8.5,"Saturday":10.0},"Tom":{"Monday":8.0}}}
                     *
                     */
                    /*
                     * getHours is the function that does the work.  It takes a message in the form of a C string that should represent a JSON Object.  The object is a message from the accounting department
                     *
                     * It immediately instantiates a COMap from the message and checks that its valid and that it contains a string object named "to" that is equal to "receiving" to verify the message is for the
                     * receiving department.  if it is it will process it.  Otherwise it will return an error message.
                     *
                     * The request is processed by first looking for a string called "request" and then checking it to see what they are requesting.  In this simple example there are to possible requests, hours
                     * which is asking for the "hours" of a number of people listed in a "people" object as a named numbers (the values will be ignored).  Or the number of hours for "info" of a single person given
                     * in a string named "employee""
                     */

                    static std::string *getHours( const char *msg )
                    {
                    	COMap		rqst( msg );																					// Create an instance of a COMap from the message
                    	COString	*sPtr;																							// Used as a temporary pointer to a COString object
                    	std::string	*reply = NULL;																					// The reply string

                    	/*
                    	 * Start by making sure the message could be istatnitated as a COMap object and that it contains a string named "to" and equals "receiving";
                    	 */
                    	if( CppON::isMap( &rqst ) && CppON::isString( sPtr = (COString *) rqst.findCaseElement( "to" ) ) && 0 == strcasecmp( sPtr->c_str(), "receiving" ) )
                    	{
                    		COMap		rsp( "{\"from\":\"receiving\"}"  );															// create the basic object that will be uses to create the response
                    		std::string request;																					// This will hold the request string pulled from the message.

                    		if( CppON::isString( sPtr = (COString *) rqst.findCaseElement( "from" ) ) )								// Check who the message was from and if found, then address the response to them
                    		{
                    			rsp.append( "to", new COString( sPtr->c_str() ) );
                    		}
                    		if( CppON::isString( sPtr = (COString *) rqst.findCaseElement( "request" ) ) )							// Look for the request string in the message to determine what the sender wants
                    		{
                    			request = sPtr->c_str();
                    		}
                    		if( ! request.compare( "hours" ) )																		// Do they want the hours for a list of people? (This could have been an array but we want
                    		{																										// to demonstrate the "extract" method.
                    			COMap		*people = (COMap *) rqst.extract( "people" );											// It's a request for hours so there should be a "peoples" object.  Extract it from the
                    																												// message so we can add it to the response and it won't be destroyed when the rqst is destroyed
                    			COMap   	*info = (COMap *) CppON::parseJsonFile( "./hours.json" );								// Read the hours file into a CppON object and cast it to a COMap;
                    			COMap		*hours;																					// This will be the "hours" object in the file that was read

                    			/*
			                     * Verify that the "people" object was found in the message and that it was a object
	                    		 * also verify that the data read from the file was a valid JSON object and that it contains a "hours" object
			                     */
			                    if( CppON::isMap( info ) && CppON::isMap( people ) && CppON::isMap( hours = (COMap *) info->findElement( "hours" ) ) )
			                    {
				                    for( std::map<std::string, CppON *>::iterator it = people->begin(); people->end() != it; it++ ) // OK, all the data looks good iterate through the people and add their hours
				                    {																								// The object name will be the persons name and the value should be a value of type CODouble
					                    COMap *h = (COMap*) hours->findCaseElement( it->first.c_str() );							// Look for the person in the hours we read from the file
			                    		/*
			                    		 * The following is done because we chose to extract the object from the message and we need to verify it is initialized to the right type and value
				                    	 */
				                    	if( CppON::isDouble( it->second ) )															// But before we do anything make sure the value we extracted from the message is a double and it
			                    		{																							// is initially set to 0.0
				                    		*( (CODouble *) it->second ) = 0.0;
				                    	} else {
		                    				delete it->second;																		// if it isn't a double then delete it and replace it with a CODouble set to 0.0
				                    		it->second = new CODouble( 0.0 );
				                    	}
				                    	((CODouble *) it->second)->Precision( 2 );													// Make sure the precision is set to 2 because we don't need 10 decimal places.

			                    		if( h )																						// Make sure the person was listed in the file and has hours
			                    		{
	                    					for( std::map<std::string, CppON *>::iterator ith = h->begin(); h->end() != ith; ith++ ) // Iterate through the days the person has hours for and add them to the total
				                    		{
                    							if( CppON::isNumber( ith->second ) )
					                    		{
                    								*( ( CODouble *) it->second ) += ( ith->second->toDouble() );
					                    		}
						                    }
                    					}
			                    	}
				                    delete( info );																					// We don't need the data from the file anymore so delete to free up the resources
							                    																					// (The static parseJsonFile() method uses the new operator)
			                    	rsp.append( "people", people );																	// Since we "extracted" the "people" object from the "rqust" object we now own it and
		                    																										// can append it to the response object (Yes! it would have been simpler to just create
				                    																								// s new one, but this was done for demonstration purposes.
                    			} else {
			                    	rsp.append( "response", new COString( "Request failed: Hours file is corrupt" ) );
			                    }
	                    	} else if( ! request.compare( "info" ) ) {																// If the request was for "info" it is for just one person listed as "employee" in the request
			                    if( CppON::isString( sPtr = (COString *) rqst.findCaseElement( "employee" ) ) )						// Look for the "employee" string.  It will be the name of the person
		                    	{
				                    COMap   	*info = (COMap *) CppON::parseJsonFile( "./hours.json" );							// Load the hours information
		                    		COMap		*employee;
			                    	std::string path( "hours/" );																	// We are going to build a path to use to get the hours information to show a different way to do it
				                    double		total = 0.0;

			                    	path.append( sPtr->c_str() );																	// We should be able to get the hours object by searching for the object in the info object by
						                    																						// "hours.<name>" where <name> is the employee name.
				                    if( CppON::isMap( info ) && CppON::isMap( employee = (COMap *) info->findCaseElement( path.c_str() ) ) ) // Look for user information in file
	                    			{																								// If found then add up the persons ours in our local variable "total"
			                    		for( std::map<std::string, CppON *>::iterator ith = employee->begin(); employee->end() != ith; ith++ )
				                    	{
						                    if( CppON::isNumber( ith->second ) )
					                    	{
                    							total += ( ith->second->toDouble() );
					                    	}
                    					}
				                    }
                    				CODouble	*d;
				                    rsp.append( "response", employee = new COMap() );												// Create a new Map object to hold the response and add it to the response
			                    	employee->append( sPtr->c_str(), d = new CODouble( total ) );									// Add the employee hours to the map object in the response
				                    d->Precision( 2 );																				// Set the precision to 2 digits
			                    	delete info;
			                    } else {
				                    rsp.append( "response", "No employee given" );
	                    		}
		                    } else {
			                    request.insert( 0, "Requested item not known: " );
		                    	rsp.append( "response", new COString( request ) );
		                    }
		                    reply = rsp.toCompactJsonString();																		// create a compact string representation of the object (Note: the rsp object is deleted
		                    																										// when it goes out of scope )
	                    } else {
		                    reply = new std::string( "{\"from\":\"receiving\",\"response\":\"Invalid message request\"}");
                    	}

                    	return reply;
                    }

                    int main( int argc, char **argv )
                    {
	                    std::string msg( "{\"to\":\"receiving\",\"from\":\"accounting\",\"request\":\"hours\",\"people\":{\"Alice\":0,\"Fred\":0,\"Mary\":0,\"Sam\":0,\"Tom\":0.0}}" );
                    //	std::string msg( "{\"to\":\"receiving\",\"from\":\"accounting\",\"request\":\"info\",\"employee\":\"Alice\"}" );
	                    std::string *hours = getHours( msg.c_str() );
	                    if( hours )
	                    {
		                    fprintf( stderr, "Hours: %s\n", hours->c_str() );
		                    delete( hours );
                    	} else {
		                    fprintf( stderr, "Failed to get hours\n" );
                    	}

                    	return 0;
                    }

   Class Libraries: There are three shared class libraries as described below:

             CPPON: This is the C++ Object Notation library that represents a JSON Message as a C++ class hierarchy.  It consists of a base class, two
                    container classes and five base object classes:
                         • C++ Object Notation Object (CppON ) - The base class from which all other classes are derived. It instantiates common methods 
                           (which are often overridden) that all objects are required to have.  It enables container classes to use the base class as the 
                           storage type.  It can be instantiated from another CppON to make a copy, created programmatic-ally, parsed from a C String, a 
                           std::string, a CSV C string a TSV C string, a JSON File, an XML file or a TNetString.
                        • C Object Map (COMap) -  This object is based on the std::Map object and is of the type std::map<std::string, CppON *>.  Obviously,
                          it is the fundamental container class that most JSON messages are based on. Besides being instantiated as a result of one of the
                          base class methods, it can be explicitly instantiated from another COMap, a JSON string representing a map or a reference to a 
                          std::map of the type <std::string, CppON *>.
                        • C Object Array (COArray) – This object is based on the std::Vector object and is of the type std::vector<CppON *>. It implements
                          the JSON Array type and is used to hold a list of objects, including other container classes.  Note that unlike C type Arrays the
                          objects don’t have to be of the same type.  Like the COMap there is a begin() and end() method which returns an iterator pointer
                          which can be used like in any vector to iterate through all the object in the container.  Likewise the at( unsigned int) function
                          can be used to retrieve an element based on an index.
                        • C Object Integer(COInterger) – This class is used to represent whole numbers which can be of size 8, 16, 32, or 64 bits in length.
                        • C Object Double (CODouble) – This class represents all floating point numbers.  Internally all floating point numbers are stored
                          as doubles but they can be retrieved as either a float value or a double value.
                        • C Object Boolean (COBoolean) – This class represents a boolean value of true or false, 1 or zero.
                        • C Object String (COString) – While the name implies correctly that this object holds a array of characters it can also hold an
                          array of binary data and/or base64 encoded binary data.  However, its main function is to save character strings.
                        • C Null Object (CONull) – This is a special case of an object.  It is basically a place holder for an undefined CPPON value.

           SCppObj: This is companion class that is dependent on the CppON class.  It is instantiated directly from a COMap object or a C String that
                    represents a path to a file which contains a string representation of one.  Externally, this class looks much like a COMap class but
                    it exists in system shared memory and has built in protection to be thread safe. (It does however have a mechanism for allowing an
                    application to control the data access on its own to insure that related data is always updated and retrieved as a block.  This data
                    structure is always defined JSON file/string with very specific rules for defining each element in the structure.  It’s an important
                    distinction that the JSON configuration file defines the data structure. The data structure is not a copy of the JSON file.

                    The constructor used to instantiate this class requires both a description JSON string or file path and a segment name string. (The
                    segment name is just a string that is used to distinguish between different SCppObj Objects in shared memory.  The constructor can 
                    also be passed a pointer to a boolean values which will be set true if the segment was previously configured/initialized false if 
                    
                    this is the first time.  Or, it could be passed a “callback” function that will be caused to initialize the structure after it is 
                    created.

                    The idea here is that all applications when they start will substantialize the object.  If it doesn’t already exist then the first
                    one to attempt to access it will create it and then initialize it, blocking all others until it is done.

                    The description JSON data contains default values for all the data but in some cases additional initialization may be required. Thus 
                    the callback function.
                    
      LocalCppObj:  The Local C++ Object is the last of the companion classes and it requires both the SCppObj class and the CppON class.  In fact it 
                    requires that a pointer to a SCppObj object be passed to the constructor.  It uses that pointer to access the data.  The calling
                    software can get pointers to sections of the data to request a response object of anything that changed since the last request.  
                    That object could be as restricted as a single base object or as broad as the whole data structure. (Although one would hope it 
                    would be something somewhere in the middle.) 

Dependent Libraries:The main dependent third party library that is required for this class family is the Jansson library used for parsing JSON strings
                    https://github.com/akheron/jansson.  Include the jansson library in your program build

                    The other libraries that are required are almost always available automatically with your distribution and are std::string, 
                    std::vector and std::map.

                    Finally, if you want to use XML with the libraries you will need to install libxml2-dev and then find the line “#define HAS_XML 0” and 
                    the CppON.hpp file and set it to 1.  Add libxml2 as a library in your build.
 Class Definitions: All the class object in the CPPON library are derived from the CppON base object and many of the methods are overridden most of them 
                    will give the correct results when cast to the base class as they are when retrieved from one of the container classes.  However, it
                    is recommended that upon obtaining a pointer to a CppON object that it be cast to the correct type when operating on/with it.

                    CppON( cppON &jt ) – Create a copy of another CppON object by references.

                    CppON( cppON *jt ) – Create a copy of another CppON object by a pointer.

                    CppON( CppONType typ=UNKNOWN_CPPON_OBJ_TYPE ); Create an empty CppON of a given type.

                    CppON( data = NULL; type=UNKNOWN_CPPON_OBJ_TYPE, precision=-1 ) – Create a CppON object of a given type while passing it the data 
                    objet. (Care should be used when doing this.)

                    factory( CppON &jt ) - Static function used to create a copy of a CppON object based on a reference.

                    factory( CppON *jt ) - Static function used to create a copy of a CppON object based on a pointer to an object.

                    type() - return the type or class name of the object.

                    size() - return the “size” of the value.  This is a virtual function and may be the size of the storage space, the number of 
                    characters or the number of items in a container.

                    dump( FILE *fp ) - This is a virtual function that is used to write the contents of the object to a file. It is written in a
                    verbose, human readable way.

                    cdump( FILE *fp ) - this is a virtual function that does a “compact” or compressed dump of an object to a file without unnecessary
                    white spaces characters in it.

                    toCompactJsonString() - This is much like the cdump() method but returns a pointer to an allocated std::string object which must be
                    deleted using the delete command.

                    getData() - Returns a pointer to the allocated data area for the object.

                    toDouble() - Attempts to convert the data to a double value and return it.  If it isn’t possible the value of -999999999.123 is 
                    returned.

                    toInteger() - Attempts to convert the data to an integer value.  If it isn’t possible then zero is returned.

                    toBoolean() - Return a boolean value of true if the value is anything but zero or false or is the ford “true” in the case of a 
                    string.
 
                    isNumber() - Returns true if the object type is a CODouble, COInteger or COBoolean
  
                    isNumber( CPPON * ) - Static method that returns true if the given object type is a CODouble, COInteger or COBoolean.
  
                    isMap() - Returns true if the object type is a COMap.
         
                    isMap( CPPON *) - Static method that returns true if the given object type is a COMap.
         
                    isArray() - Returns true if the object type is a COArray.

                    IsArray( CPPON * ) - Static method that returns true if the given object type is a COArray.

                    isString() - Returns true if the object type is a COString.
 
                    isString( CPPON * ) - Static method that returns true if the given object type is a COString.
 
                    isBoolean() - Returns true if the object type is a COBoolean.
 
                    isBoolean( CPPON * ) - Static method that returns true if the given object type is a COBoolean.
 
                    isInteger() - Returns true if the object type is a COInteger.
 
                    isInteger( CPPON * ) - Static method that returns true if the given object type is a COInteger.
 
                    isDouble() - Returns true if the object type is a CODouble.
 
                    isDouble( CPPON * ) - Static method that returns true if the given object type is a CODouble.

                    isObj( CPPON * ) - Static function that returns true if the given object is defined as a specific type
 
                    == - Returns true if the two values are the same.

                    != - Returns true if the two values are not the same.

                    diff( CppON &obj, const char *name ) - Returns a CppON containing the differences between the object and the given one.  Good for
                    finding changes in a map or an array.

                    c_str() - This returns a const char pointer to a C string representing the data.  This could be just the conversion of a number to
                    a string or in the case of a COMap or COArray it could be the complex JSON string.

                    readObj( FILE *fp ) - read the file and attempt to parse the contents into a CPPON object.

                    parse( const char *str, char *rstr ) - Parse a TNetString and attempt to create a CPPON object. The remainder of the string (Not 
                    parsed) is returned pointed to by rstr. Caution is advised as TnetStrings are often used to bass binary data that is invalid in JSON.

                    parseJson( const char *str ) - This function is used to attempt to crate a CPPON object from a C string.  It will determine the type
                    of object based on the first not white space character.

                    parseJson( json_t *ob, std::string &tabs ) - This is actually a low level function used to parse JSON data using the jansson library.
                    Users should ignore it.

                    parseXML( const char *str ) – This function (available if the HAS_XML flag is set) is used to attempt to create a CPPON object from
                    a string containing XML.

                    parseCSV( const char *file ) - This function is used to attempt to parse data in CSV file pointed to by file into a COArray.  
 
                    parseTSV( const char *file ) - This function is used to attempt to parse data in TSV file pointed to by file into a COArray.  

                    parseJsonFile( const char *file ) - This function is used to attempt to parse data in file pointed to by file into a CPPON object.  

                    GuessDataType( const char *str ) - Given a sting representing an unknown JSON data type try and parse it ito a CPPON Object.
 
         COInteger: The COInteger class can represent whole values of different sizes.  8, 16, 32, or 64 bits.  While it overrides many of the methods in
                    the base class, while more efficient the operation remains the same. However there are a few methods added specifically for this class.

                    COInteger( X ) - Constructors passing different types of values.  Char, short, int, long, long long types define the size of the
                    COInteger  as well as set the value of the object.

                    = operator is used to set the size and value of the COInteger based on the data type and value of the number it is set to.

                    longValue() - Returns the value, no matter what size it is, as a long long.

                    charValue() - Returns the value, no matter what size it is, truncated to an eight bit character.

                    shortValue() - Returns the value, no matter what size it is, truncated to a sixteen bit number.

                    intValue() - Returns the value, no matter what size it is, truncated to a 32 bit number.

                    toNetString() - creates and returns a std::string pointer to a TNetString representation of the value. (Should to be deleted)

                    toJsonString() - creates and returns a std::string pointer to a JSON String representation of the value. (Should to be deleted)

          CODouble: Used to represent floating point numbers, it uses the “precision” parameter to determine the number of significant digits to 
                    represent when converted to a string.  The default precision is ten digits.

                    CODouble( double d = 0.0 ) - Has the additional constructor which allows it to be constructed from a double precision number.

                    Precision( unsigned char n ) - Allows the precision to be set (if n is given ) and returned as an unsigned character.

                    = operator allows the value to be set to a given double value or the value of a CODouble.

                    set( const double &d ) -  Allows the value to be set in a method. (same as = ).

                    doubleValue() - Returns the value as a double precision number.

                    floatValue() - Returns the value as a single precision float number.  

                    toNetString() - creates and returns a std::string pointer to a TNetString representation of the value. (Should to be deleted)

                    toJsonString() - creates and returns a std::string pointer to a JSON String representation of the value. (Should to be deleted)

            CONull: While the CONull class exists it is only a “transitional” object and doesn’t provide any purpose except for completeness.

         COBoolean: This class represents boolean values of true and false.

                    = operator provides a number of ways the false can be set from either another COBoolean or a c++ bool value.  And, while might 
                    be good to add the ability to set it to a zero or non zero number, this has not been done.

                    Value() - The value function returns a boolean value of true or false. 

                    toNetString() - creates and returns a std::string pointer to a TNetString representation of the value. (Should to be deleted)

                    toJsonString() - creates and returns a std::string pointer to a JSON String representation of the value. (Should to be deleted)

          COString: This class is implement as a std::string object.  Unlike JSON strings it allows binary data to be stored in in them.
 
                    COString( const char *st = "", bool base64 = false ) - This constructor is used to construct a COString from a const char pointer.
                    This pointer
                    can be either a normal C string or a base64 encoded C string which will be decoded before storing it.  

                    COString( std::string st, bool base64 = false ) - This constructor is used to construct a COString from a std::string object.  The
                    std::string can contain either normal ASCII characters or be a base64 included string of binary data which will be decoded before
                    storage.

                    COString( uint64_t val, bool hex = true ) - This constructor is used to convert a 64 bit number into a COString object.  It can be
                    stored in decimal format or as a Hex representation.

                    COString( uint32_t val, bool hex = true ) - This constructor is used to convert a 32 bit number into a COString object.  It can be
                    stored in decimal format or as a Hex representation.

                    Base64Decode( const char *tmp, unsigned int sz, unsigned int &len, char *out = NULL ) - This function is more or less a generic 
                    function used to convert a Base 64 encoded string back into binary data.  It is used by the class and is made available to other
                    applications.  “tmp” is a pointer to the base 64 data and sz is the size of the data.  “out” if set must point to a buffer large
                    enough to hold the data ( sz + 3 should work).  If it is NULL then space will be allocated as an array of characters with the new
                    operator. “len” will be set to the number of bytes in the output string.  Which will be null terminated even though it is binary
                    data?

                    = operator works for setting the value to both C strings and std::strings but also integers of type uint64_t, uint32_t, and int.

                    toBase64JsonString( const char *data, unsigned int len ) - static member function to base64 Encode binary data int a JSON string
                    where data is a pointer to the binary data and len is the number of characters.

                    ToBase64JsonString() - method to convert a COString that holds binary data into a JSON string of base64 characters.

                    toNetString() - creates and returns a std::string pointer to a TNetString representation of the value. (Should to be deleted)

                    toJsonString() - creates and returns a std::string pointer to a JSON String representation of the value. (Should to be deleted)

             COMap: This is the main container class which usually holds all other instances of the message.  It is implemented as a 
                    std::map<std::string, CPPON *>.

                    As such it has a number of functions dedicated to iterating through the items in the map as well as the ability to search add
                    and remove items from from the map.  It is important to note that all objects contained in the map are “owned” by the map.  This
                    means that when the object is destroyed all object contained by the map will also be destroyed.
          
                    It should also be understood that when a COMap object is created either by the use of a JSON string or by a JSON Object all object
                    contained will also be created.  In this way a very complicated JSON string or object can be created with a single constructor.

                    COMap( const char *str ) - Create a class hierarchy based on a JSON string representing a map and child objects.

                    COMap( std::map<std::string, CPPON *> &m ) - Create a copy of an existing map object containing CPPON objects.

                    = operator can be used to create a COMap from either a JSON string or another COMap object.

                    getKeys() - Will return a pointer to a vector of type std::vector<std::string> which contains all the keys in the map. (Do NOT
                    delete or modify the returned vector.  The vector is actually part of the class and is used to key track of the order in which
                    the elements are added.

                    getValues() - Will return a pointer to a vector of type std::vector<CppON *> representing the values stored in the Object. Don’t
                    delete the objects in in the vector but the vector itself is created by the call and must be deleted.
 
                    value() - returns a pointer to a std::map< std::string, CppON *>.  This is the actual map that the object is built on. Do not add
                    or delete object directly from the map.
 
                    begin() - returns an iterator of type std::map<std::string,CppON*>::iterator which is pointing to the first element in the map.
                    This iterator can be used to index through the child objects in the map.
 
                    end() - returns an iterator of type std::map<std::string, CppON *>::iterator which points to the first object after the end of
                    the map which can be used as a comparisons to the one return from the begin() function to know that the all values have be indexed
                    through.

                    extract( const char *name ) - Search for a element that string matches the “name”.  If found remove it and return a pointer to it.
                    Note, as it has been removed from the COMap, the code is responsible for deleting it.

                    removeVal( std::string val ) - This method looks for an object by the given name.  If found it is removed and deleted.

                    replaceObj( std::string name, CppON *obj ) - Look for an object by the given name.  If found it is deleted and replaced by the
                    given object.

                    clear() - Remove and delete all objects from the COMap.
                    
                    append( std::string key, CppON *n ) - This is the typical function to add a new object to the COMap object.  The name and object
                    to add are passed to the method.
 
                    append( std::string key, std::string value ) - Create a new COString object from the value and add it to the COMap object.
 
                    append( std::string key, const char *value ) - Create a new COString object from the value and add it to the COMap object.
 
                    append( std::string key, double value ) - Create a new CODouble object from the value and add it to the COMap object.

                    append( std::string key, long long value ) - Create a new COInteger object from the value and add it to the COMap object.

                    append( std::string key, int value ) - Create a new COInteger object from the value and add it to the COMap object.


                    append( std::string key, bool value ) - Create a new COBoolean object from the value and add it to the COMap object.

                    append( std::string key ) - Create a new CONull object and add it to the COMap object.

                    findEqual( const char *name, CppON *) - Look for an object with the name and equal to a give Object.

                    findElement( const char *str ) - Find and return a pointer to an element by looking for the name.  Note that if str has a period (.)
                    or a forward slash (/) in it, it is considered a “path” with the dot or slash being a delimiter.  And the name is considered the
                    first segment before the delimiter. If the item is found and it is a COMap type the first segment is removed from the str and the
                    shortened name is passed to to the new object to be found.  If the item is an COArray the delimiter is a colon (:) followed by a
                    number which is an index.

                    findElement( const char *str ) - Find and return a pointer to an element by looking for the name.  In this case the name is not
                    considered a “path” and must be equal to the name in the map.

                    findElement( const std::string &str), findElement( const char *str ), findElement( std::string *s ) - Same as the findElement( const
                    char *str) method.

                    findCaseElement( const char *str ), findCaseElement( const std::string &str ), findCaseElement( const std::string *str) – Same as find
                    Element, but ignore the case of the characters while doing the search.

                    toNetString() - creates and returns a std::string pointer to a TNetString representation of the value. (Should to be deleted)

                    toJsonString() - creates and returns a std::string pointer to a JSON String representation of the value in human readable form.
                    (Should to be deleted)

                    toCompactJsonString() - creates and returns a std::string pointer to a JSON String representation of the value with no additional
                    white space characters inserted. This makes it ideal for sending as a message. (Should to be deleted)

                    upDate( COMap *map, const char *name ) - This is used to update a COMap with data from another COMap.  The name allows the update
                    operation to be performed on an object in the map rather than the map as a whole.

                    merge( COMap *map, const char *name ) - Used to combine two COMaps into one.

                    diff( COMap *newObj, const char *name ) - used to produce a COMap off all the objects that are different between two COMaps.

           COArray: This Object is used to simulate a JSON Array object.  They aren’t used all that often but support is needed.  The COArray is
                    implemented as a std::vector< CppON *>.  Like with the COMap object this class supports iterators and paths.

                    COArray( std::vector< CppON *> &v ) - This constructor is used to create a COArray from just a vector of CppON object pointers.

                    value() - Return a pointer to the actual vector that holds the data.

                    begin() - return a std::vector< CppON *>::iterator that points to the first element in the vector.

                    end() - return a std::vector< cppON *>::iterator that represents the end of the vector.

                    append( CppON *n ) - Add an object to the vector.

                    append( std::string value ) - Create a COString object from value and append it to the vector.

                    append( double value ) - Create a CODouble object from value and append it to the vector.

                    append( long long value ) - Create a COInteger object from value and append it to the vector.
  
                    append( int value ) - Create a COInteger object from the value and append it to the vector.

                    append( bool value ) - Create a COBoolean object from the value and append it to the vector.

                    push_back( CppON *n ) - Append a CppOn object to the end of the vector.

                    push( CppON *n ) - The same as push_back, it adds an object to the end of the vector.
 
			  pop( ) - Remove and return the last item in the vector.
        
                    pop_front() - Remove and return the first item in the vector.

                    replace( size_t i, CppON *n) – remove and destroy the element at index i and replace it with the given object. 
               
                    remove( size_t idx) – Remove and return the element at index idx.

                    at( unsigned int i ) - Return a pointer to the element at index i.

                    Clear() - remove and destroy all objects from the vector.

                    toNetString() - creates and returns a std::string pointer to a TNetString representation of the value. (Should to be deleted)

                    toJsonString() - creates and returns a std::string pointer to a JSON String representation of the value in human readable form.
                    (Should to be deleted)

                    toCompactJsonString() - creates and returns a std::string pointer to a JSON String representation of the value with no additional
                    white space characters inserted. This makes it ideal for sending as a message. (Should to be deleted)
          
           SCppObj: This is a “companion class” of the CPPON class.  It is dependent on it but adds some real functionality when using the CPPON class
                    for use as “soft data structures”.  It allows the construction of such memory structures in system shared memory to be access by
                    many programs and to be persistent even when the creating application shuts down or is restarted. 

                    The SCppObj.cpp class is an extension to the CppON class in that it requires the class in its initialization and to function.

                    Its purpose is to create and maintain a shared memory object that can be used by multiple applications and persist over shutdowns
                    and startups on those applications.

                    The idea is that the object data structure is defined by a JSON file, and behaves much like an instance of a CPPON hierarchy,
                    referenced by a given name and can be created by any application.

                    There are a couple of features that are important to understand with the SCppObj.  The first is that like a JSON object (or CppON)
                    all data is referenced by string path names where the data is referenced like in JSON as objects, arrays, strings, doubles, integers
                    (Long of int) or bool (boolean) but unlike a CppON the data is not stored in ASCII or in actual structures of objects. Also, once
                    created the structure is fixed. Space is allocated in shared memory based on information in the description. Strings have a maximum
                    length.

                    Path names can be delimited by either a dot "." or forward slash "/".  There is no difference.

                    The JSON description file:
                    
                    When instantiated the first parameter of the SCppObj constructor is either a path name to a JSON file or a CppON::COMap.  This JSON 
                    object is a description file that defines the layout of the SCppObj that is created.  It is organized as a COMap object containing
                    other COMap objects which represent JSON objects.  Each object describes either a data type which could be a container object or one
                    of the basic types: string, float, bool, or int.  The data type is defined by a string value (which is required) called "type".  For
                    a object type the type value is set to "unit", and will be implemented as a "COMap" containing other objects.  Another required value
                    for strings and integers is the "size" integer value. This give the number of bytes allocated for the data. (It's not required for
                    booleans, nulls, or floats because the space for those is fixed. For strings the size it the maximum number of characters allocated 
                    plus one for the null character.  For the integer types it must be 8, 4, 2 or 1.

                    For all basic types the "defaultValue" must be defined which is the initial value of the field on creation.  And, for the "float" 
                    type a "precision" value can be defined which indicates the number of significant digits when converting it to a string.
 
                    To summarize: The object is defined by a structure of structures with each structure representing a JSON value.  The value can be a
                    type "COMap" which holds other JSON data and is denoted by the "type": "unit".  Or, it can be a basic type and denoted by the 
                    "type": "string" | "float" | "int" | "bool".  "int" and "string" types must have a "size" parameter and "floats" can have a 
                    "precision" value.  All basic values must have a defaultValue specified which the object is set to in initialization.
 
       The SCppObj:

                    Besides being stored in system shared memory, the SCppObj is far more efficient then storing data in either a CppON or JSON strings.  This
                    is because the offset to the data is fixed and known.

                    However this comes at a cost:  Once created, it's structure if fixed and cannot be changed.  Strings have a fixed maximum length.  Of course
                    another great thing about it is the data is stored in system shared memory and is only be deleted by a reboot or performing the shared memory
                    library call: shm_unlink( sm_name );  Where sm_name is the name of the SCppObj. (But the system will only delete the shared memory segment
                    when all references to it are closed.)  All applications on the system can access it without fear of thread collisions or corruption.  This
                    is due to its use of system semaphores.  These are handled by the class, but when desired to maintain synchronization of data can be handled 
                    by the application.
 
                    It's important to note that the data is not stored in structures nor is accessible without accessing it through path names. When an application
                    first instantiates the SCppObj object it builds an internal network of pointers called "STRUCT_LISTS" which are references to all objects and
                    where they are stored in memory.  These Object contain type information and pointers to all contents and data that the variable accesses.  You
                    can get a reference to one of these objects with the "getElement" request but should NEVER directly modify the contents one of these objects.
 
                    Each data type and integer sizes have their own "accessors" to get and update data in the structure.  There are three types of accessors,  An
                    example of each is:
                    	*  bool updateString( const char *path, const char *str, bool protect = true, STRUCT_LISTS *lst = NULL ); ( returns true if value is valid )
                    	*  double *readDouble( const char *path, double *result = NULL, bool protect = true, STRUCT_LISTS *lst = NULL );
                     	*  double Doublevalue( const char *path, bool protect= true, STRUCT_LISTS *lst = NULL, bool *valid = NULL )
 
                    With all data access calls the first parameter in the call defines the element the operation is to be performed on.  This can be a 
                    "STRUCT_LISTS" pointer to the element (not usually used) or a path name to the element. (To avoid lengthy searches it is best to use relative
                    addressing of data.)  The path name starts at the top level and transitions through the object tree based on a dot (.) or slash (/) delimited
                    string.  In the case of a "update" or "read" accessor call, the second parameter is always the value or pointer to the value to be returned.
                    "Value" accessors don't have this parameter as the value is returned from the method.

                    For character array only, "read" calls the next parameter is the max size of the space to store the data.
                    
                    The next parameter is a boolean value where: true means to use (checkout) the semaphore for the structure and false means to ignore the 
                    semaphore for this operation. (Make sure that if you enter true, the default, that you haven't already obtained the semaphore.  Remember: there
                    is just one semaphore for all the data in the same structure.) Its a best practice to never "hold" more then one semaphore at a time and to only
                    do so for a minimum amount of time. Making sure that you always return it.
 
                    The next optional parameter can be a "Parent" STRUCT_LISTS * which the first parameter is relative to.  The use of this can be a big performance
                    boost as finding the data pointer is much faster.  (The default is NULL, and the "path" must be relative to the root)
 
                    On "value" accessors there is another optional parameter which is a point to a boolean value.  If the pointer is non-NULL (default is NULL) the
                    return value will be true if the value is found, meaning the returned value is valid.
 
                    There are three possible constructors
                       1) SCppObj::SCppObj( COMap *def, const char *segmentName, bool *initialized );
                       2) SCppObj::SCppObj( const char *configPath, const char *segmentName, bool *initialized )
                       3) SCppObj::SCppObj( const char *configPath, const char *segmentName, void(*f)( SCppObj *obj ) )
                    
                    The first two are the same, differing only in the fact that the fist is given a CppON::COMap for its configuration while the second is given a 
                    pathname of a JSON file that contains the configuration.   The second parameter is the segment name which will identify the shared memory 
                    segment and the third optional parameter is a pointer to a boolean flag so the calling application can determine if the instantiation of the 
                    object was the one that created and initialized it, in case more needs to be done.
 
                    The third constructor allows you to pass a callback routine in the form of "void p( SCppObj *object );" which will be called if the object is 
                    first created to allow a program to do further initialization that isn't contained in the JSON file.  (NOTE: as there usually isn't any 
                    guarantee which application first instantiates thus creating the object, all application should use this form if it is required that the data 
                    altered by the initiation callback.
 
                    This object can produce very complex multi-level data structures in a very optimized storage space.  It protects access of each structure buy
                    use of semaphores, one for each structure.
                    
                    Semaphores are used by default for all accesses, both read or write.  However this functionality can be overridden on a call by call basis.  
                    Also, for performance reasons you can "capture" the semaphore for a structure, performs multiple access to data in the structure without using 
                    them and then release it.

                    Another big performance benefit when accessing data in a single structure is that a reference to the structure itself can be retrieved and than
                    accesses to data in it can be done relative to it.  I.E  Suppose you have a structure that is multiple levels deep in the configuration like 
                    system/configuration/tsp which contain the configuration information for the tracking signal processor.  With a SCppObj called "sysdata" you 
                    can do something like the following:
                        uint64_t          hardwareRev = 0;
                        int32_t           softwareRev = 0;
                        std::string       address("");
                        char              amazonId[ 18 ];
                        bool              hasTSP = false;
                        bool              valid = false;
 
                    STRUCT_LISTS *tsp = sysdata->getElement( "configuration.tsp" );            // Get a reference object to the tsp section of the configuration data.
                                                                                               // (Note: Could of used "configuration/tsp" )
                    if( tsp )	                                                                 // Make sure your request was successful. ( tsp is a reverence don't 
                                                                                               // delete it. )
                    {
                        sysdata->waitSem( "address", tsp );	                                    // Optional: Capture the semaphore for the whole tsp section so values
                                                                                               // can be changed in a group and increase performance
                        sysdata->readString( "address", &address, false, tsp );                // read a string value "configuration.tsp.address" into a std::string
                                                                                               // value called "address" without getting the semaphore and using the tsp 
                                                                                               // reference
                        sysdata->readString( "amazon_number, amazonId, 18, false, tsp );	      // read a string value into a character array of size 18 (size must include
                                                                                               // null character)
                        sysdata->readBool( "installed", &hasTSP, false, tsp );                 // read a boolean value called installed into a value hasTSP
                        stsdata->readInt( "software_rev", &softwareRev, false, tsp );          // read an 32 bit integer called software_rev
                        hardwareRev=sysdata->longValue( "hardware_rev", false, tsp, &valid );  // Another way to read a value.  This time returning the value.  The 
                                                                                               // optional parameter "valid" is set to true if the return value is valid.
                                                                                               // I.E. the "configuration.tsp.hardware_rev" 8 byte integer exists.
                        sysdata->updateString( "user", "Peter", false, tsp );                  // modify the string value "configuration/tsp/user" to make it contain 
                                                                                               // "Peter\0"
                        address = "192.168.123.156";
                        sysdata->updateString( "address", address, false, tsp);                // modify the string value "configuration/tsp/address" to make it contain
                                                                                               // "192.168.123.156\0"

                        sysdata->postSem( "address, tsp );                                     // Free the semaphore so other threads can access the data.
                    }

                    Like with the CppON there are calls to determine the type like isInteger, isDouble, isString ....
                    There are also a whole bunch of calls to return a CppON value of the various types (To prevent memory leaks these should be deleted.)   They are
                    things like:
                        * toCODouble() returns a CODouble
                        * toCOInteger() return a COInteger
                        * toCOMap() returns a COMap of a structure
                        * ToJObject() returns an object of whatever type it is.
                        * and so forth
 
                    There are other functions which you can look at the header file for but this pretty much covers what you need.

                    Semaphore use:
                    Semaphores are a necessary evil of multi-threaded programming.  The protect data from corruption and reading invalid or un-synchronized data.  
                    Especially strings.  However they are extremely dangerous if misused because the can cause threads to become permanently locked.  Although in its
                    basic form the CObj can be used without every dealing with them, in which case it will handle them safely without intervention.  Or access can be 
                    used in such a way as to ignore the semaphores on a call by call basis.  Considerable performance and safety can be gained by a little 
                    management on the part of the programmer.  To avoid problems there are a few basic rules that must always be followed when dealing with them:
                        1) Never own two semaphores at the same time.  This is just looking for trouble  especially when one thread can have one and then try to check
                           out another while another thread has the second on and is try to check out the first.
                        2) Be very careful never to try to check out the same semaphore twice. This is a guaranteed thread lock.
                        3) When you check out a semaphore, get done quickly and free it up as soon as possible.  Avoid calling subroutines or methods while you own
                           a semaphore.
                        4) Don't checkout semaphores from within an interrupt.
                        5) Be very careful of multiple paths or exits from code after checking out a semaphore which could cause the code to not free it.

                    Best practices:
                       * The shared memory object can be very large and complicated but usually a section of code will only be working within a certain area.  In
                         which case it is always best to get a reference to the "piece" of the object and access the information relative to it.  You can do this 
                         with or without owning the semaphore to the structure.
                       * If you know that your thread is the only one modifying a section of data, you can bypass the semaphore checking on reads.

	
