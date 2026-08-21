/**___________________________________________________________________________/
@file          Json.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Json implementation. The parsing of json is done recursively which means we run
the risk of stack overflow, so will need to be changed eventually

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "json.h"

namespace {
	void tabPadding(std::stringstream& ofs, int stack) {
		for (int i{}; i < stack; i++) {
			ofs << '\t';
		}
	}

	// This will be adding backslashes infront of characters such as \ ' and "
	void stringParse(std::string& s) {
		std::stringstream sstr;
		for (const char& c : s) {
			if (c == '\\' || c == '\'' || c == '"') {
				sstr << '\\';
			}
			sstr << c;
		}
		s = sstr.str();
	}
}

json::value& json::value::operator=(const json::value& v) {

	type = v.type;
	isVec = v.isVec;

	if (v.data != nullptr) {

		if (isVec) {
			data = new std::vector<value>{ *static_cast<std::vector<value>*>(v.data) };
		}
		else {
			switch (v.type) {
			case DATATYPE::FLOAT: {
				data = new float{ *static_cast<float*>(v.data) };
				break;
			}
			case DATATYPE::INT: {
				data = new int{ *static_cast<int*>(v.data) };
				break;
			}
			case DATATYPE::STRING: {
				data = new std::string{ *static_cast<std::string*>(v.data) };
				break;
			}
			case DATATYPE::OBJECT: {
				data = new object{ *static_cast<object*>(v.data) };
				break;
			}
			case DATATYPE::BOOL: {
				data = new bool{ *static_cast<bool*>(v.data) };
				break;
			}
			case DATATYPE::DOUBLE: {
				data = new double{ *static_cast<double*>(v.data) };
				break;
			}
			case DATATYPE::UINT: {
				data = new unsigned int{ *static_cast<unsigned int*>(v.data) };
				break;
			}
			case DATATYPE::ULL: {
				data = new unsigned long long{ *static_cast<unsigned long long*>(v.data) };
				break;
			}
#if _DEBUG
			default:
				std::cout << "UNREGISTERED CASE!!!! <COPY ASSIGNMENT>";
#endif
			}
		}

	}

	return *this;
}

json::value::value(const value& v) {
	*this = v;
}

json::value::value(value&& v) noexcept : data{ v.data }, isVec{ v.isVec }, type{ v.type } {
	v.data = nullptr;
}

json::value& json::value::operator=(value&& v) noexcept {
	void* tmp = v.data;
	v.data = data;
	data = tmp;
	isVec = v.isVec;
	type = v.type;
	return *this;
}

json::value::~value() {
	if (isVec) {
		delete static_cast<std::vector<value>*>(data);
		return;
	}

	switch (type) {
	case DATATYPE::EMPTY:
		break;
	case DATATYPE::INT:
		delete static_cast<int*>(data);
		break;
	case DATATYPE::FLOAT:
		delete static_cast<float*>(data);
		break;
	case DATATYPE::DOUBLE:
		delete static_cast<double*>(data);
		break;
	case DATATYPE::UINT:
		delete static_cast<unsigned int*>(data);
		break;
	case DATATYPE::BOOL:
		delete static_cast<bool*>(data);
		break;
	case DATATYPE::STRING: {
		delete static_cast<std::string*>(data);
		break;
	}
	case DATATYPE::OBJECT: {
		delete static_cast<object*>(data);
		break;
	}
	case DATATYPE::ULL: {
		delete static_cast<unsigned long long*>(data);
		break;
	}
#if _DEBUG
	default:
		std::cout << "UNREGISTERED CASE!!! <DESTRUCTOR>" << " type: " << type << '\n';
#endif
	}
}

json::json(const object& obj) {
	container.push_back(obj);
}

json::json(const std::string& s) : container{} {
	std::stringstream sstr = CEO::Instance().GetManager<FileManager>()->ReadFile(s);
	char c{};

	if (sstr.str().empty()) {
		std::cout << "Couldn't find file \"" << s << "\"\n";

		// could add some debug thing here maybe
	}

	while (sstr.good()) {
		c = static_cast<char>(sstr.get());
		// everything starts as an object right erm
		if (c == '{') {
			object obj;

			deserializeObject(sstr, obj);

			// could this be slow? since obj could be a pretty big well, object...
			container.emplace_back(std::move(obj));
		}

		if (sstr.eof())
		{
			return;
		}
	}
}

void json::deserializeObject(std::stringstream& sstr, object& obj, bool isPair, bool isArray) {
	pair p = std::make_pair(std::string(), value());

	char c{}; bool ParsingName{ false };
	bool BackspaceChar{ false };
	std::stringstream name;

	// Parsing the name of the object
	// Won't parse if its part of an array
	if (!isArray) {


		while (sstr.good()) {
			c = static_cast<char>(sstr.get());
			if (c == '"' && !ParsingName) {
				ParsingName = true;
				continue;
			}

			if (ParsingName) {
				if (c == '\\' && !BackspaceChar) {
					BackspaceChar = true;
					continue;
				}
				if (BackspaceChar) {
					switch (c) {
					case 'n':
						name << '\n';
						break;
					case '\\':
						name << '\\';
						break;
					case 't':
						name << '\t';
						break;
					case '?':
						name << '\?';
						break;
					case 'v':
						name << '\v';
						break;
					case '\'':
						name << '\'';
						break;
					case 'b':
						name << '\b';
						break;
					case '"':
						name << '"';
						break;
					case 'r':
						name << '\r';
						break;
					case '0':
						name << '\0';
						break;
					case 'f':
						name << '\f';
						break;
					default:
						name << c; // sum ting wong
					}

					// no octal/hex parsing 

					BackspaceChar = false;
				}
				else if (c == '"') {
					p.first = name.str();
					break;
				}
				else {
					name << c;

				}


			}

			// Empty object check
			if (c == '}' && !ParsingName) {
				return;
			}
		}
	}

	// Parsing values
	bool ParsingString{ false }, ParsingNumber{ false }, ParsingObject{ false }, ParsingArray{ false }, ParsingBool{ false }, Parsing{};
	bool newPair{ false };
	bool Float{ false }, Double{ false }, Unsigned{ false }; int Long{ 0 };
	BackspaceChar = false;
	std::stringstream stringvalue;
	std::vector<value> arrayVector;
	object arrayObject; // dummy object to be used to store parsed value

	while (sstr.good()) {
		c = static_cast<char>(sstr.get());

		// skipping the space like characters
		if (!ParsingString) {
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
				continue;
		}

		Parsing = ParsingString || ParsingNumber || ParsingObject || ParsingArray || ParsingBool;

		// another object
		if (c == '{' && !Parsing) {
			ParsingObject = true;
		}

		// Array
		if (c == '[' && !Parsing) {
			ParsingArray = true;
		}

		// Empty array check
		if (c == ']' && !Parsing && isArray) {
			sstr.unget();
			return;
		}


		// string
		if (c == '"' && !Parsing) {
			ParsingString = true;
			// Has continue because " is not part of the string
			continue;
		}

		// number
		if (((c >= '0' && c <= '9') || c == '-') && !Parsing) {
			ParsingNumber = true;
		}

		// boolean
		if ((c == 't' || c == 'T' || c == 'f' || c == 'F') && !Parsing) {
			ParsingBool = true;
		}


		// ----- string parsing ----- //
		if (ParsingString) {
			if (c == '\\' && !BackspaceChar) {
				BackspaceChar = true;
				continue;
			}
			if (BackspaceChar) {
				switch (c) {
				case 'n':
					stringvalue << '\n';
					break;
				case '\\':
					stringvalue << '\\';
					break;
				case 't':
					stringvalue << '\t';
					break;
				case '?':
					stringvalue << '\?';
					break;
				case 'v':
					stringvalue << '\v';
					break;
				case '\'':
					stringvalue << '\'';
					break;
				case 'b':
					stringvalue << '\b';
					break;
				case '"':
					stringvalue << '"';
					break;
				case 'r':
					stringvalue << '\r';
					break;
				case '0':
					stringvalue << '\0';
					break;
				case 'f':
					stringvalue << '\f';
					break;
				default:
					stringvalue << c; // sum ting wong
				}

				// no octal/hex parsing 

				BackspaceChar = false;
			}
			else if (c == '"') {

				value v;

				v.data = new std::string();
				v.type = DATATYPE::STRING;

				*static_cast<std::string*>(v.data) = stringvalue.str();

				p.second = std::move(v);
				obj.vec.emplace_back(std::move(p));

				ParsingString = false;

				c = static_cast<char>(sstr.peek());
				// More values in the object
				if (c == ',') {
					newPair = true;
				}
				else {
					break;
				}

			}
			else {
				stringvalue << c;
			}

		}



		// ----- number parsing ----- //
		if (ParsingNumber) {
			if ((c < '0' || c > '9') && c != '-' && c != '+' && c != 'e' && c != 'E' && c != '.'
				&& c != 'u' && c != 'U' && c != 'd' && c != 'D' && c != 'f' && c != 'F' && c != 'l' && c != 'L') {
				bool temp{ false };

				// Edge case stuff
				if (c == ',') temp = true;
				if (c == ']' || c == '}') sstr.unget();

				value v;
				if (Double) {
					v.data = new double;
					v.type = DATATYPE::DOUBLE;

					*static_cast<double*>(v.data) = std::stod(stringvalue.str());
				}
				else if (Float) {
					v.data = new float;
					v.type = DATATYPE::FLOAT;

					*static_cast<float*>(v.data) = std::stof(stringvalue.str());
				}
				else {
					if (Unsigned && Long == 2) {
						v.data = new unsigned long long;
						v.type = DATATYPE::ULL;
						*static_cast<unsigned long long*>(v.data) = std::stoull(stringvalue.str());
					}
					else if (Unsigned) {
						v.data = new unsigned int;
						v.type = DATATYPE::UINT;
						*static_cast<unsigned int*>(v.data) = static_cast<unsigned int>(std::stoul(stringvalue.str()));
					}
					else {
						v.data = new int;
						v.type = DATATYPE::INT;

						*static_cast<int*>(v.data) = std::stoi(stringvalue.str());
					}
				}

				p.second = std::move(v);
				obj.vec.emplace_back(std::move(p));

				ParsingNumber = false;

				c = static_cast<char>(sstr.peek());
				// More values in the object
				if (c == ',' || temp) {
					newPair = true;
				}
				else {
					break;
				}

			}
			else {
				if (c == 'F' || c == 'f') Float = true;
				if (c == 'D' || c == 'd') Double = true;
				if (c == 'U' || c == 'u') Unsigned = true;
				if (c == 'L' || c == 'l') Long++;
				if (c == '.') Float = true;
				stringvalue << c;
			}
		}

		// ----- bool parsing -----//
		if (ParsingBool) {
			if (c == 'e' || c == 'E') {
				stringvalue << c;
				std::string string = stringvalue.str();

				std::transform(string.begin(), string.end(), string.begin(), [](char c) { return std::tolower(c); });

				value v;

				v.data = new bool;
				v.type = DATATYPE::BOOL;

				if (string == "true") {
					*static_cast<bool*>(v.data) = true;
				}
				else if (string == "false") {
					*static_cast<bool*>(v.data) = false;
				}
				else {
					v.data = nullptr;
				}

				p.second = std::move(v);
				obj.vec.emplace_back(std::move(p));

				ParsingBool = false;

				c = static_cast<char>(sstr.peek());
				// More values in the object
				if (c == ',') {
					newPair = true;
				}
				else {
					break;
				}

			}
			else {
				stringvalue << c;
			}
		}

		// ----- array parsing ----- //
		if (ParsingArray) {
			if (c == ']') {
				value v;

				if (!arrayObject.vec.empty())
					v.type = arrayObject.vec.at(0).second.type;
				v.isVec = true;
				v.data = new std::vector<value>;

				for (pair& pp : arrayObject.vec) {

					arrayVector.emplace_back(std::move(pp.second));
				}

				*static_cast<std::vector<value>*>(v.data) = std::move(arrayVector);
				

				p.second = std::move(v);
				obj.vec.emplace_back(std::move(p));

				ParsingArray = false;
				c = static_cast<char>(sstr.peek());
				// More values in the object
				if (c == ',') {
					newPair = true;
				}
				else {
					break;
				}

			}
			else {
				deserializeObject(sstr, arrayObject, false, true);
			}

		}



		// ----- object parsing ----- //
		if (ParsingObject) {
			value v;
			v.data = new object;
			v.type = DATATYPE::OBJECT;

			deserializeObject(sstr, *static_cast<object*>(v.data));

			p.second = std::move(v);
			obj.vec.emplace_back(std::move(p));

			c = static_cast<char>(sstr.peek());
			// More values in the object
			if (c == ',') {
				newPair = true;
			}
			else {
				break;
			}

		}

		if (newPair) {
			if (isArray) {
				deserializeObject(sstr, obj, true, true);
			}
			else {
				deserializeObject(sstr, obj, true);
			}

			break;
		}


	}

	if (isPair || isArray) {
		return;
	}

	// Finding for closing bracket

	c = static_cast<char>(sstr.get());
	while (c) {
		if (c == '}') return;
		if (sstr.eof()) return;
		c = static_cast<char>(sstr.get());
	}

	return;
}


bool json::Serialize(const std::string& s) {
	int objStackCount{ 0 };

	std::stringstream sstr;

	for (int i{}; i < container.size(); i++) {
		const object& obj = container[i];
		// some recursive function here
		serializeObject(sstr, obj, objStackCount);
		if (i != container.size() - 1) sstr << ",\n";
	}

	std::ofstream ofs(s);
	ofs << sstr.rdbuf();

	return true;
}

void json::serializeObject(std::stringstream& sstr, const object& obj, int stackCount, bool isArray) {

	if (obj.vec.empty()) {
		sstr << "{}";
		return;
	}

	if (isArray) tabPadding(sstr, stackCount);

	sstr << "{\n";
	for (int x{}; x < obj.vec.size(); x++) {

		const pair& p = obj.vec[x];

		tabPadding(sstr, stackCount + 1);
		std::string name = p.first;
		stringParse(name);
		sstr << '"' << name << '"' << ": ";

		switch (p.second.type) {
		case DATATYPE::OBJECT:
			if (p.second.isVec) {

				sstr << "[\n";


				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const object& obj2 = *static_cast<object*>((*vec)[i].data);

					serializeObject(sstr, obj2, stackCount + 2, true);
					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";
			}
			else {
				serializeObject(sstr, *static_cast<object*>(p.second.data), stackCount + 1);
			}
			break;
		case DATATYPE::INT: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const int& obj2 = *static_cast<int*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					sstr << obj2;

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				int data = *static_cast<int*>(p.second.data);
				sstr << data;
			}
			break;
		}
		case DATATYPE::UINT: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const unsigned int& obj2 = *static_cast<unsigned int*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					sstr << obj2 << 'u';

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				unsigned int data = *static_cast<unsigned int*>(p.second.data);
				sstr << data << 'u';
			}
			break;
		}
		case DATATYPE::ULL: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const unsigned int& obj2 = *static_cast<unsigned int*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					sstr << obj2 << "ull";

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				unsigned int data = *static_cast<unsigned int*>(p.second.data);
				sstr << data << "ull";
			}
			break;
		}
		case DATATYPE::FLOAT: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const float& obj2 = *static_cast<float*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					sstr << obj2 << 'f';

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				float data = *static_cast<float*>(p.second.data);
				sstr << data << 'f';
			}
			break;
		}
		case DATATYPE::DOUBLE: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const double& obj2 = *static_cast<double*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					sstr << obj2 << 'd';

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				double data = *static_cast<double*>(p.second.data);
				sstr << data << 'd';
			}
			break;
		}
		case DATATYPE::STRING: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					std::string& obj2 = *static_cast<std::string*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					stringParse(obj2);
					sstr << "\"" << obj2 << "\"";

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				std::string& data = *static_cast<std::string*>(p.second.data);
				stringParse(data);
				sstr << "\"" << data << "\"";
			}
			break;
		}
		case DATATYPE::BOOL: {
			if (p.second.isVec) {
				sstr << "[\n";

				std::vector<value>* vec = static_cast<std::vector<value>*>(p.second.data);

				for (int i{}; i < vec->size(); i++) {

					const bool& obj2 = *static_cast<bool*>((*vec)[i].data);

					tabPadding(sstr, stackCount + 2);
					if (obj2) sstr << "true";
					else sstr << "false";

					if (i != vec->size() - 1) sstr << ',';
					sstr << '\n';
				}

				tabPadding(sstr, stackCount + 1);

				sstr << "]";

			}
			else {
				bool data = *static_cast<bool*>(p.second.data);
				if (data) sstr << "true";
				else sstr << "false";
			}
			break;
		}
		case DATATYPE::EMPTY:
			if (p.second.isVec) {
				sstr << "[]";
			}
		}

		if (x != obj.vec.size() - 1) sstr << ',';
		sstr << '\n';
	}

	tabPadding(sstr, stackCount);
	sstr << '}';
}





json::value* json::GetValue(const std::string& s) {
	for (object& o : container) {
		for (pair& p : o.vec) {
			if (p.first == s) {
				return &p.second;

			}
		}
	}
	return nullptr;
}

json::object* json::GetObj(const std::string& s) {
	value* v = GetValue(s);
	if (v == nullptr) return nullptr;
	return v->GetObj();
}

std::vector<json::object*> json::GetObjVec(const std::string& s) {
	value* v = GetValue(s);
	if (v == nullptr) return std::vector<json::object*>();
	return v->GetObjVec();
}

json::object* json::object::GetObj(const std::string& s) {
	for (const pair& p : vec) {
		if (p.first == s) {
			if (p.second.type == DATATYPE::OBJECT && !p.second.isVec) {
				return static_cast<object*>(p.second.data);
			}
			return nullptr;

		}
	}
	return nullptr;
}

std::vector<json::object*> json::object::GetObjVec(const std::string& s) {
	for (const pair& p : vec) {
		if (p.first == s) {
			if (p.second.type == DATATYPE::OBJECT && p.second.isVec) {

				std::vector<value>* vec2 = static_cast<std::vector<value>*>(p.second.data);
				std::vector<object*> data;

				for (const value& v : *vec2) {
					object* obj = static_cast<object*>(v.data);
					data.push_back(obj);
				}

				return data;

			}
			return std::vector<object*>();

		}
	}
	return std::vector<object*>();
}

json::value* json::object::GetValue(const std::string& s) {
	for (pair& p : vec) {
		if (p.first == s) {
			return &p.second;

		}
	}
	return nullptr;
}

int* json::value::GetInt() {
	if (type == DATATYPE::INT && !isVec) {
		return static_cast<int*>(data);
	}
	return nullptr;
}
bool* json::value::GetBool() {
	if (type == DATATYPE::BOOL && !isVec) {
		return static_cast<bool*>(data);
	}
	return nullptr;
}
float* json::value::GetFloat() {
	if (type == DATATYPE::FLOAT && !isVec) {
		return static_cast<float*>(data);
	}
	return nullptr;
}
double* json::value::GetDouble() {
	if (type == DATATYPE::DOUBLE && !isVec) {
		return static_cast<double*>(data);
	}
	return nullptr;
}
unsigned int* json::value::GetUInt() {
	if (type == DATATYPE::UINT && !isVec) {
		return static_cast<unsigned int*>(data);
	}
	return nullptr;
}
unsigned long long* json::value::GetULL() {
	if (type == DATATYPE::ULL && !isVec) {
		return static_cast<unsigned long long*>(data);
	}
	return nullptr;
}
std::string* json::value::GetString() {
	if (type == DATATYPE::STRING && !isVec) {
		return static_cast<std::string*>(data);
	}
	return nullptr;
}
json::object* json::value::GetObj() {
	if (type == DATATYPE::OBJECT && !isVec) {
		return static_cast<object*>(data);
	}
	return nullptr;
}

std::vector<int*> json::value::GetIntVec() {
	if (type == DATATYPE::INT && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<int*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<int*>(v.data));
		}
		return propervec;
	}
	return std::vector<int*>();
}
std::vector<bool*> json::value::GetBoolVec() {
	if (type == DATATYPE::BOOL && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<bool*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<bool*>(v.data));
		}
		return propervec;
	}
	return std::vector<bool*>();
}
std::vector<float*> json::value::GetFloatVec() {
	if (type == DATATYPE::FLOAT && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<float*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<float*>(v.data));
		}
		return propervec;
	}
	return std::vector<float*>();
}
std::vector<double*> json::value::GetDoubleVec() {
	if (type == DATATYPE::DOUBLE && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<double*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<double*>(v.data));
		}
		return propervec;
	}
	return std::vector<double*>();
}
std::vector<unsigned int*> json::value::GetUIntVec() {
	if (type == DATATYPE::UINT && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<unsigned int*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<unsigned int*>(v.data));
		}
		return propervec;
	}
	return std::vector<unsigned int*>();
}
std::vector<unsigned long long*> json::value::GetULLVec() {
	if (type == DATATYPE::ULL && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<unsigned long long*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<unsigned long long*>(v.data));
		}
		return propervec;
	}
	return std::vector<unsigned long long*>();
}
std::vector<std::string*> json::value::GetStringVec() {
	if (type == DATATYPE::STRING && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<std::string*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<std::string*>(v.data));
		}
		return propervec;
	}
	return std::vector<std::string*>();
}
std::vector<json::object*> json::value::GetObjVec() {
	if (type == DATATYPE::OBJECT && isVec) {
		std::vector<value>* vec = static_cast<std::vector<value>*>(data);

		std::vector<object*> propervec;

		for (const value& v : *vec) {
			propervec.push_back(static_cast<object*>(v.data));
		}
		return propervec;
	}
	return std::vector<object*>();
}

void json::object::AddPair(const std::string& s, const int& i) {				int* data = new int{ i };								AddPairVoid(s, static_cast<void*>(data), DATATYPE::INT); }
void json::object::AddPair(const std::string& s, const unsigned int& i) {		unsigned int* data = new unsigned int{ i };				AddPairVoid(s, static_cast<void*>(data), DATATYPE::UINT); }
void json::object::AddPair(const std::string& s, const unsigned long long& i) { unsigned long long* data = new unsigned long long{ i };	AddPairVoid(s, static_cast<void*>(data), DATATYPE::ULL); }
void json::object::AddPair(const std::string& s, const float& i) {				float* data = new float{ i };							AddPairVoid(s, static_cast<void*>(data), DATATYPE::FLOAT); }
void json::object::AddPair(const std::string& s, const bool& i) {				bool* data = new bool{ i };								AddPairVoid(s, static_cast<void*>(data), DATATYPE::BOOL); }
void json::object::AddPair(const std::string& s, const std::string& i) {		std::string* data = new std::string{ i };				AddPairVoid(s, static_cast<void*>(data), DATATYPE::STRING); }
void json::object::AddPair(const std::string& s, const double& i) {				double* data = new double{ i };							AddPairVoid(s, static_cast<void*>(data), DATATYPE::DOUBLE); }
void json::object::AddPair(const std::string& s, const object& i) {				object* data = new object{ i };							AddPairVoid(s, static_cast<void*>(data), DATATYPE::OBJECT); }

void json::object::AddPair(const std::string& s, const std::vector<int>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (int j : i) {
		value v;
		v.data = new int{ j };
		v.type = DATATYPE::INT;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::INT, true);
}
void json::object::AddPair(const std::string& s, const std::vector<unsigned int>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (unsigned int j : i) {
		value v;
		v.data = new unsigned int{ j };
		v.type = DATATYPE::UINT;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::UINT, true);
}
void json::object::AddPair(const std::string& s, const std::vector<float>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (float j : i) {
		value v;
		v.data = new float{ j };
		v.type = DATATYPE::FLOAT;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::FLOAT, true);
}
void json::object::AddPair(const std::string& s, const std::vector<bool>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (bool j : i) {
		value v;
		v.data = new bool{ j };
		v.type = DATATYPE::BOOL;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::BOOL, true);
}
void json::object::AddPair(const std::string& s, const std::vector<std::string>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (const std::string& j : i) {
		value v;
		v.data = new std::string{ j };
		v.type = DATATYPE::STRING;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::STRING, true);
}
void json::object::AddPair(const std::string& s, const std::vector<double>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (double j : i) {
		value v;
		v.data = new double{ j };
		v.type = DATATYPE::DOUBLE;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::DOUBLE, true);
}
void json::object::AddPair(const std::string& s, const std::vector<object>& i) {
	std::vector<value>* data = new std::vector<value>;

	for (const object& j : i) {
		value v;
		v.data = new object{ j };
		v.type = DATATYPE::OBJECT;

		data->emplace_back(std::move(v));
	}

	AddPairVoid(s, static_cast<void*>(data), DATATYPE::OBJECT, true);
}

void json::object::AddPairVoid(const std::string& s, void* data, DATATYPE type, bool isVec) {

	value v;
	v.data = data;
	v.type = type;
	v.isVec = isVec;

	pair p;
	p.first = s;
	p.second = std::move(v);

	vec.emplace_back(std::move(p));
}

bool json::object::RemovePair(const std::string & s) {
	vec.erase(std::remove_if(vec.begin(), vec.end(), [&s](pair& pair) {
		if (pair.first == s) {
			return true;
		}
		return false;
	}));
	return false;
}