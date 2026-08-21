/**___________________________________________________________________________/
@file          demo.h
@author        j.junbo@digipen.edu
@date          9/29/2025

demo on json and error

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "demo.h"
#include <iostream>
#include <vector>
#include "InputManager.h"
#include "Components.h"

namespace DEMO {
	void json_demo() {
		// A json object is initialized with the filepath to the json file.
		// It will automatically deserialize it and store the data within the object
		json j("Assets/test.json");

		// The underlying storage of the json class is a vector of "objects".
		// You would use "GetObj" or "GetObjVec" to access the data.
		// GetValue is for if there is a pair at the outermost layer

		// GetValue example
		std::string* sStr = j.GetValue("lonely pair")->GetString();
		std::cout << "\nOpening pair: " << *sStr << '\n';

		// GetObj example
		json::object* sObj = j.GetObj("sample object");

		// The way you would extract the stored data from the object is by 
		// using GetValue + Get[type] ( i.e. GetInt() ). Since I store the 
		// data as a * void in the backend, different functions are used to 
		// cast the variables to the correct type before passing it over to
		// the user. 
		// You could refer to the test.json to see what I am getting from 
		// the file

		// Getting an int
		int* sInt = sObj->GetValue("item1")->GetInt();
		// If the typing is wrong, or a value doesnt exist, it returns a nullptr.
		if (sInt) std::cout << "the item1 is: " << *sInt << '\n';

		// You could also store unsigned integers, you will just have to add
		// an "U" or "u" just like standard C++ to specificy unsigned
		unsigned int* sUint = sObj->GetValue("item2")->GetUInt();
		if (sUint) std::cout << "the item2 is: " << *sUint << '\n';

		// If the number has a decimal place in it, the default storage is
		// a float, just like GLM. But just like C++, you can specify 
		// double/float using the "d","D"/"f","F" specifiers. 
		// unsigned double/float does not exist last I checked so do not
		// combine those thanks.
		float* sFloat = sObj->GetValue("item3")->GetFloat();
		double* sDouble = sObj->GetValue("item4")->GetDouble();
		if (sFloat) std::cout << "the item3 is: " << *sFloat << '\n';
		if (sDouble) std::cout << "the item4 is: " << *sDouble << '\n';
		
		// Lastly booleans can be stored too, as true or false (no
		// quotation marks needed). You can also just capitalize them
		// anyhow, so TRUE and TruE and true are all the same to my 
		// deserializer.
		bool* sBool = sObj->GetValue("item5")->GetBool();
		if (sBool) std::cout << "the item5 is: " << *sBool << '\n';

		// If the values are stored in an array, you need to use the 
		// Get[Type]Vec version ( i.e. GetIntVec() ). It will return a
		// vector of pointers.
		// Do note: If you pop back/push back the returned vector, it will
		// not affect what is stored within the json class. 
		std::vector<int*> sIntVec = sObj->GetValue("item6")->GetIntVec();
		std::cout << "the item6 is: ";
		for (int* i : sIntVec) {
			// For the vectors, on failure it will return an empty vector 
			// instead, so there is no need for a nullpointer check here.
			std::cout << *i << " ";
		}
		std::cout << '\n';

		int* demo = new int{ 10 };
		sIntVec.push_back(demo); // This does nothing!!
		sIntVec.pop_back(); sIntVec.pop_back(); // This also does nothing!!

		delete demo; // no memleaks allowed here

		// Objects could also be nested, and so could the getters. You 
		// could instead of saving the intermediaries just chain them like 
		// this:
		std::string* sStr2 = sObj->GetObj("item7")->GetObj("nested items")->GetValue("can nest even more")->GetString();
		std::string* sStr3 = sObj->GetObj("item7")->GetObj("nested items")->GetObj("but please")->GetValue("dont")->GetString();
		if (sStr2) std::cout << "the nested item7 has: " << *sStr2 << ' ';
		if (sStr3) std::cout << "and: " << *sStr3 << '\n';

		// To edit a value all we have to do is just change the value of 
		// the pointer, thats it.
		if(sInt) *sInt = 15; // Done!

		// testing ull implementation
		unsigned long long* sUll = sObj->GetValue("item8")->GetULL();
		std::cout << "ULL value: " << *sUll << '\n';

		// To add a key/value pair, use the addPair on an object type, wont 
		// work on values.

		sObj->AddPair("new item", 12.34); // Note since C++ default considers 
										  // this as a double, you will need to
										  // add an "f" to make it store as a float

		// If you wish to use a C-Style raw string, there is a seperate function
		// as the compiler will choose the boolean overload by default when it 
		// sees a const char*
		sObj->AddPairRawString("for strings", "use this", 8);

		// Casting to string works too
		sObj->AddPair("this also", (std::string)"works too");


		// This shows how it could be used within the scene manager context

		// "entities" contains an arry of objects so you need to use GetObjVec
		std::vector<json::object*> i = j.GetObjVec("entities");
		
		// Lets say we want to do something to the guy with ID 1:
		for (json::object* obj : i) {
			int* ID = obj->GetValue("ID")->GetInt();

			if (ID && *ID == 1) {

				// Now lets say we want to find the name and change it

				std::string* name = obj->GetObj("Name Component")->GetValue("name")->GetString();
				if (name) std::cout << "Old name: " << *name << '\n';
				*name = "now i have this funny name";
				if (name) std::cout << "New name: " << *name << '\n';

				// Presume that ID:1 unfortunately met an accelerated end

				bool* active = obj->GetObj("Active Component")->GetValue("Active")->GetBool();
				*active = false;

			}
		}


		// You can use serialize to write the data back into the file. You
		// can give it the same filepath as before. I have it as different
		// so you can see the before and after of the editions we made.
		j.Serialize("Assets/test3.json");
	}

	void error_demo() {
		try {
			ErrorLog& el = ErrorLog::Instance(); // getting instance for easier typing

			// Initializing random variables
			int i{ 5 };
			float f{ .5f };
			double d{ 1.4 };
			std::string s("hello there");

			int* pi{ &i };
			float* pf{ nullptr };

			std::vector<int> vi;

			// Very important!! 
			el.SetFunction("error_demo");

			// Has two initialization requirements:
			// 1: Must have the exact same name as the setfunction
			// 2: Must be initialized AFTER everything else
			// Since it is initialized last, it's destructor will be hte first to be called,
			// and it is the destructor that will be calling the errorlog's snapshot function
			// that saves a snapshot of all of the logged variables.
			ErrorLog::Catch c("error_demo");

			// Adding a new print for int vectors
			ErrorLog::Instance().AddAnyPrint<std::vector<int>>([](const std::vector<int>& v) {
				std::cout << "Type: std::vector<int>, Values: ";
				if (v.empty()) std::cout << "Empty";
				else {
					for (int i : v) {
						std::cout << i << ' ';
					}
				}
				std::cout << '\n';
				});

			// Logging of variables
			el.Log("i", &i); el.Log("f", &f); el.Log("d", &d);
			el.Log("s", &s); el.Log("pi", &pi); el.Log("pf", &pf);
			el.Log("vi", &vi);

			// imagine you do some stuff here
			i++;
			f *= static_cast<float>(d);
			*pi += 5;


			// Throwing an exception when the key "c" gets pressed.
			(void)vi.at(3);

			// we do a little more here. These wont get logged as the exception happens before
			i = 15;
			f -= 2.f;
			pf = &f;
		}
		catch (...) {
			ErrorLog::Instance().PrintLog();
		}

	}

}