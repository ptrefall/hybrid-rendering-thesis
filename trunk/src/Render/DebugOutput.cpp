#include "DebugOutput.h"

#include <iostream>
#ifdef WIN32
#include <intrin.h>
#endif

using namespace Render;

DebugOutput::DebugOutput()
{
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(&DebugOutput::DebugOutputToConsole, nullptr);
#endif 

	/*GLuint MessageId(4);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 1, &MessageId, GL_FALSE);
	std::string Message1("This is a test");
	glDebugMessageInsertARB(
		GL_DEBUG_SOURCE_APPLICATION_ARB, 
		GL_DEBUG_TYPE_OTHER_ARB, 1, 
		GL_DEBUG_SEVERITY_LOW_ARB,
        GLsizei(Message1.size()), Message1.c_str());*/
}

DebugOutput::~DebugOutput()
{
}

void DebugOutput::DebugOutputToConsole(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam)
{
	char debSource[256], debType[256], debSev[256];
    if(source == GL_DEBUG_SOURCE_API_ARB)
        strcpy(debSource, "OpenGL");
    else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
        strcpy(debSource, "Windows");
    else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
        strcpy(debSource, "Shader Compiler");
    else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
        strcpy(debSource, "Third Party");
    else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)
        strcpy(debSource, "Application");
    else if(source == GL_DEBUG_SOURCE_OTHER_ARB)
        strcpy(debSource, "Other");
 
    if(type == GL_DEBUG_TYPE_ERROR_ARB)
        strcpy(debType, "Error");
    else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
        strcpy(debType, "Deprecated behavior");
    else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
        strcpy(debType, "Undefined behavior");
    else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)
        strcpy(debType, "Portability");
    else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
        strcpy(debType, "Performance");
    else if(type == GL_DEBUG_TYPE_OTHER_ARB)
        strcpy(debType, "Other");
 
    if(severity == GL_DEBUG_SEVERITY_HIGH_ARB)
        strcpy(debSev, "High");
    else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
        strcpy(debSev, "Medium");
    else if(severity == GL_DEBUG_SEVERITY_LOW_ARB)
        strcpy(debSev, "Low");
 
	std::cout << "Source: " << debSource << "\tType: " << debType << "\tID: " << id << "\tSeverity: " << debSev << "\tMessage: " << message << std::endl;
    //printf("Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource,debType,id,debSev,message);
#ifdef WIN32
	if(type == GL_DEBUG_TYPE_ERROR_ARB || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB || severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		__debugbreak();
#endif
}

