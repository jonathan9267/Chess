#ifndef SHADER
#define SHADER

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader{
  public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath){
      std::string vertexCode;
      std::string fragmentCode;
      std::ifstream vShaderFile;
      std::ifstream fShaderFile;

      vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
      fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

      try{
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
      } catch(std::ifstream::failure e){
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ";
      }
      const char* vShaderCode = vertexCode.c_str();
      const char* fShaderCode = fragmentCode.c_str();

      unsigned int vertex, fragment;

      int success;
      char infoLog[512];

      //Vertex shader
      vertex = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex, 1, &vShaderCode, NULL);
      glCompileShader(vertex),

      checkIfSuccess(vertex, &success, infoLog);

      //Fragment shader

      fragment = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment, 1, &fShaderCode, NULL);
      glCompileShader(fragment);

      checkIfSuccess(vertex, &success, infoLog);

      ID = glCreateProgram();
      glAttachShader(ID, vertex);
      glAttachShader(ID, fragment);
      glLinkProgram(ID);

      glGetProgramiv(ID, GL_LINK_STATUS, &success);
      if (!success){
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << infoLog;
      }
      else{
        std::cout << "Shaders succesfully linked.\n";
      }

      glDeleteShader(vertex); 
      glDeleteShader(fragment);
    };

    void use(){
      glUseProgram(ID);
    }
    void setBool(const std::string & name, bool value) const{
      glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
    }
    void setInt(const std::string &name, int value) const{      
      glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
    }
    void setFloat(const std::string &name, float value) const{      
      glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
    }
    void checkIfSuccess(unsigned int vertex, int* success, char *infoLog){
      glGetShaderiv(vertex, GL_COMPILE_STATUS, success);
      if (!success){glGetShaderInfoLog(vertex, 512, NULL, infoLog); std::cout << infoLog;}
    }
};
 
#endif

