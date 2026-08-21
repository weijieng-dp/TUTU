#pragma once
/**___________________________________________________________________________/
@file          Json.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Homemade Json serializer/desrializer.

  Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include "FileManager.h"


/*!
* \brief
*   Json serializer/deserializer. Initialize the object with a filepath to deserialize a file
* \brief
*	Use member function Serialize() to serialize it into a file.
*/
class json {
public:
	enum DATATYPE {
		EMPTY,
		INT,
		UINT,
		FLOAT,
		DOUBLE,
		STRING,
		BOOL,
		ULL,
		OBJECT
	};

	struct value;

	using pair = std::pair<std::string, value>;
	struct object;

public:


	/*!
	* \brief
	*   Constructor for the json object
	*
	* \param 
	*	[std::string&] the filepath to the json file
	*/
	json(const std::string& filepath);

	/*!
	* \brief
	*   Conversion constructor from an object
	*
	* \param
	*	[object&] A json::object type
	*/
	json(const object& obj);

	json() = delete; // removing default constructor

	/*!
	* \brief
	*   Serializes the data into the filepath
	*
	* \param
	*	[std::string&] the filepath to serialized file
	* 
	* \return
	*	[bool] true on success, false on failure
	*/
	bool Serialize(const std::string& filepath);

	//Getters
	
	/*!
	* \brief
	*   Gets a pointer to a json::value object
	*
	* \param
	*	[std::string&] the name of the value in the json file
	*
	* \return
	*	[value*] a pointer to the value object. Returns a nullptr on failure.
	*/
	value* GetValue(const std::string& s);

	/*!
	* \brief
	*   Gets a pointer to a json::object object
	*
	* \param
	*	[std::string&] the name of the value in the json file
	*
	* \return
	*	[object*] a pointer to the json::object object. Returns a nullptr on failure.
	*/
	object* GetObj(const std::string& s);

	/*!
	* \brief
	*   Gets a vector of pointers to a json::object object
	*
	* \param
	*	[std::string&] the name of the value in the json file
	*
	* \return
	*	[std::vector<object*>] a vector containing pointers to json::object objects. Returns an empty vector on failure.
	*/
	std::vector<object*> GetObjVec(const std::string& s);

	std::vector<object>& Container() { return container; }


	/*!
	* \brief
	*   A struct encapsulation to store "any" type
	*/
	struct value {
	public:
		DATATYPE type{ EMPTY };					// enum to store what type is being stored
		bool isVec{ false };					// boolean for if it is a vector

		value() : data{ nullptr } {}			// default constructor
		value(const value& v);					// copy constructor
		value& operator=(const value& v);		// copy assignment
		value(value&& v) noexcept;				// move constructor
		value& operator=(value&& v) noexcept;	// move assignment

		~value();								// destructor

		/*!
		* \brief
		*   Get the underlying int data 
		*
		* \param null
		*
		* \return
		*	[int*] an int pointer to the data. Returns nullptr on failure
		*/
		int* GetInt();

		/*!
		* \brief
		*   Get the underlying bool data
		*
		* \param null
		*
		* \return
		*	[bool*] a bool pointer to the data. Returns nullptr on failure
		*/
		bool* GetBool();

		/*!
		* \brief
		*   Get the underlying float data
		*
		* \param null
		*
		* \return
		*	[float*] a float pointer to the data. Returns nullptr on failure
		*/
		float* GetFloat();

		/*!
		* \brief
		*   Get the underlying double data
		*
		* \param null
		*
		* \return
		*	[double*] a double pointer to the data. Returns nullptr on failure
		*/
		double* GetDouble();

		/*!
		* \brief
		*   Get the underlying unsigned int data
		*
		* \param null
		*
		* \return
		*	[unsigned int*] an unsigned int pointer to the data. Returns nullptr on failure
		*/
		unsigned int* GetUInt();

		/*!
		* \brief
		*   Get the underlying unsigned long long data
		*
		* \param null
		*
		* \return
		*	[unsigned long long*] an unsigned long long pointer to the data. Returns nullptr on failure
		*/
		unsigned long long* GetULL();

		/*!
		* \brief
		*   Get the underlying string data
		*
		* \param null
		*
		* \return
		*	[string*] a string pointer to the data. Returns nullptr on failure
		*/
		std::string* GetString();

		/*!
		* \brief
		*   Get the underlying object data
		*
		* \param null
		*
		* \return
		*	[object*] an object pointer to the data. Returns nullptr on failure
		*/
		object* GetObj();



		/*!
		* \brief
		*   Get the underlying int array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<int*>] a vector containing int pointers to the data. Returns an empty vector on failure
		*/
		std::vector<int*> GetIntVec();

		/*!
		* \brief
		*   Get the underlying bool array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<bool*>] a vector containing bool pointers to the data. Returns an empty vector on failure
		*/
		std::vector<bool*> GetBoolVec();

		/*!
		* \brief
		*   Get the underlying float array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<float*>] a vector containing float pointers to the data. Returns an empty vector on failure
		*/
		std::vector<float*> GetFloatVec();

		/*!
		* \brief
		*   Get the underlying double array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<double*>] a vector containing double pointers to the data. Returns an empty vector on failure
		*/
		std::vector<double*> GetDoubleVec();

		/*!
		* \brief
		*   Get the underlying unsigned int array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<unsigned int*>] a vector containing unsigned int pointers to the data. Returns an empty vector on failure
		*/
		std::vector<unsigned int*> GetUIntVec();

		/*!
		* \brief
		*   Get the underlying unsigned long long array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<unsigned long long*>] a vector containing unsigned long long pointers to the data. Returns an empty vector on failure
		*/
		std::vector<unsigned long long*> GetULLVec();

		/*!
		* \brief
		*   Get the underlying string array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<std::string*>] a vector containing string pointers to the data. Returns an empty vector on failure
		*/
		std::vector<std::string*> GetStringVec();

		/*!
		* \brief
		*   Get the underlying object array data in the form of a vector
		*
		* \param null
		*
		* \return
		*	[std::vector<object*>] a vector containing object pointers to the data. Returns an empty vector on failure
		*/
		std::vector<object*> GetObjVec();

	private:
		void* data;
		friend class json;
	};

	struct object {
	public:
		object() : vec{} {}

		/*!
		* \brief
		*   Gets a pointer to the value linked to a key
		*
		* \param 
		*	[std::string&] the name of the value in the json file
		*
		* \return
		*	[value*] the json::value object linked to the key. Returns a nullptr on failure.
		*/
		value* GetValue(const std::string& s);

		/*!
		* \brief
		*   Gets a pointer to the object linked to a key
		*
		* \param
		*	[std::string&] the name of the object in the json file
		*
		* \return
		*	[object*] the json::object object linked to the key. Returns a nullptr on failure.
		*/
		object* GetObj(const std::string& s);

		/*!
		* \brief
		*   Gets a vector containing pointers to objects linked to a key
		*
		* \param
		*	[std::string&] the name of the value in the json file
		*
		* \return
		*	[value*] the json::value object linked to the key. Returns a nullptr on failure.
		*/
		std::vector<object*> GetObjVec(const std::string& s);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[int&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const int& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[unsigned int&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const unsigned int& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[unsigned long long&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const unsigned long long& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[float&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const float& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[bool&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const bool& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::string&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::string& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[double&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const double& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[object&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const object& o);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<int>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<int>& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<unsigned int>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<unsigned int>& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<float>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<float>& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<bool>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<bool>& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<std::string>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<std::string>& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<double>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<double>& i);

		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[std::vector<object>&] the value of the pair
		*
		* \return null
		*/
		void AddPair(const std::string& s, const std::vector<object>& i);


		/*!
		* \brief
		*   Adds a Key/Value pair to the object
		*
		* \param
		*	[std::string&] the name of the value, or the "Key"
		* \param
		*	[const char*] the value of the pair in c-style raw string
		* \param 
		*	[int] the length of the raw string
		*
		* \return null
		*/
		void AddPairRawString(const std::string& s, const char* ch, int length)
		{ std::string string; string.insert(0, ch, length); AddPair(s, string); }
		
		
		bool RemovePair(const std::string& s);

	private:

		void AddPairVoid(const std::string& s, void* value, DATATYPE type, bool isVec = false);
		std::vector<pair> vec;
		friend class json;
	};

private:

	// Assumes its an object by default. Can be used for arrays too
	void deserializeObject(std::stringstream& sstr, object& obj, bool isPair = false, bool isArray = false);
	void serializeObject(std::stringstream& sstr, const object& obj, int stackCount, bool isArray = false);

	std::vector<object> container;
};