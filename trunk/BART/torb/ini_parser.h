#include <cstdio>
#include <fstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype> // isspace
#include <functional>
#include <glm/glm.hpp>

namespace ini
{
	typedef std::unordered_map<std::string, std::string> PropertyValueMap;
	struct section_t
	{
		std::string sectionName;
		PropertyValueMap values;
	};
		
	// trim from start
	static inline std::string &ltrim(std::string &s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
			return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
			return s;
	}
	
	// trim from both ends
	static inline std::string &trim(std::string &s) {
			return ltrim(rtrim(s));
	}
	
	std::string eraseAfterFirst(std::string str, char c)
	{
		size_t idx = str.find_first_of(c);
		if (idx != -1) {
			str.erase(idx);
		}
		return str;
	}
	
	class Parser
	{
		std::vector<section_t> sections;
		std::string filepath;

		public:
		Parser(const std::string& filepath) : filepath(filepath)
		{
			parse();
		}
			
		bool getBool( const char* section, const char* property, bool defaultValue )
		{
			return getProperty<bool>( section, property, defaultValue );
		}
		
		int getInt( const char* section, const char* property, int defaultValue )
		{
			return getProperty<int>( section, property, defaultValue );
		}
		
		float getFloat( const char* section, const char* property, float defaultValue )
		{
			return getProperty<float>( section, property, defaultValue );
		}
		
		std::string getString( const char* section, const char* property, const std::string& defaultValue )
		{
			return getProperty<std::string>( section, property, defaultValue );
		}
				
		template <typename T> 
		bool getProperty( const char* section, const char* property, bool defaultValue )
		{
			std::string val = getValue(section, property);
			if ( val != "" )
			{
				std::transform( val.begin(), val.end(), val.begin(), tolower );
				
				if ( val == "true" ) {
					return true;
				} else {
					return false;
				}
			}
			return defaultValue;
		}
		
		template <typename T> 
		T getProperty( const char* section, const char* property, T defaultValue )
		{
			std::string val = getValue(section, property);
			if ( val != "" )
			{
				std::stringstream sstream;
				sstream << val;
				T tmp;
				sstream >> tmp;
				return tmp;
			}
			return defaultValue;
		}
				
		glm::vec2 getVec2( const char* section, const char* property, glm::vec2 defaultValue )
		{
			return getVec<glm::vec2>( section, property, defaultValue );
		}
		glm::vec3 getVec3( const char* section, const char* property, glm::vec3 defaultValue )
		{
			return getVec<glm::vec3>( section, property, defaultValue );
		}
		glm::vec4 getVec4( const char* section, const char* property, glm::vec4 defaultValue )
		{
			glm::vec4 v;
			return getVec<glm::vec4>( section, property, defaultValue );
		}
		
		template <typename T>
		T getVec( const char* section, const char* property, T defaultValue )
		{
			std::string val = getValue(section, property);
			if ( val != "" ) {
				T v(0);
				size_t components = sizeof(T)/sizeof(float);
				std::stringstream sstream;
				sstream << val;
				for(size_t i=0; i<components; i++){
					sstream >> v[i];
				}
				return v;
			}
			return defaultValue;
		}
		
		void print()
		{
			std::cout << "sections for " << filepath << std::endl;
			for(auto it=sections.begin(); it!=sections.end(); ++it){
				std::cout << (*it).sectionName << std::endl;
				
				const PropertyValueMap &values = (*it).values;
				
				for ( auto it2=values.begin(); it2!=values.end(); ++it2  ){
					std::cout << '\t' << (*it2).first << "=" << (*it2).second << std::endl;
				}
			}
		}
	
		private:
			
		std::string getValue( const char* section, const char* property )
		{
			for(auto it=sections.begin(); it!=sections.end(); ++it)
			{
				if ( (*it).sectionName == section ) {
					
					const PropertyValueMap &values = (*it).values;
					
					auto mapIt = values.find(property);
					if ( mapIt != values.end() ) {
						return (*mapIt).second;
					}
						
				}
			}
			return "";
		}
		
		void createSection( std::ifstream& in, const std::string& sectionName )
		{
			section_t sec;
			sec.sectionName = sectionName;
			sections.push_back( sec );
			
			// look for value names
			std::string str;
			while( std::getline(in,str) )
			{
				str = eraseAfterFirst(str, ';'); // Remove comment
				if ( str.size() == 0 ) continue; // first char was a comment

				if ( str[0] == '[' )
				{
					for(size_t i=0; i<str.size()+1; i++){
						in.unget(); // found a new section. go back 1 line
					}
					return; 
				}
				else if( str[0] == ' ' || str[0] == ';' ) 
				{
					// ignore empty lines
				}
				else
				{
					size_t pos = str.find_first_of('='); // property = value
					if ( pos != -1 ){
						std::string propName = str.substr(0, pos); // property
						std::string value = str.substr(pos+1); // value
						
						propName = trim(propName);
						value = trim(value);	
						
						section_t &sec = sections.back();
						sec.values[propName] = value;
					}
					
				}
			}
		}
		
		void parse()
		{
			std::ifstream in( filepath );
			
			// look for a section
			std::string str;
			while( std::getline(in,str) )
			{
				if ( str[0] == '[' )
				{
					std::string sectionName = str.substr(1, str.size()-2 );
					createSection( in, sectionName );
				}
			}
		}	
	
	};

} // end namespace