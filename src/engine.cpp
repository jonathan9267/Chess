#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "shaders/shader.h"
#include <dependencies/textures/stb_image.h>
#include "textures.h"
#include "inputevents.h"
#include "mathUtil.h"

#define VERTEX_SIZE 3
#define START_X -1
#define START_Y -1
#define RECT_INDICES_SIZE 6
#define normalizedLength 2

void framebuffer_size_callback(GLFWwindow*, int, int);

class Engine{

    struct Vector2{double x; double y;};
    float increment;
    const int columns = 8;
    const int rows = 8;
    //To draw squares on a column, row grid we need vertices on the edge of the grid. 
    const int boardSize = (columns+1)*(rows+1)*VERTEX_SIZE;
    //We need 6 vertices (2 triangles) for every square.
    const int indicesSize = columns*rows*RECT_INDICES_SIZE;
    public:
        Engine(int width, int height){

            this->width = width;
            this->height = height;
            //Increments as a fraction of total width/height.
            this->increment= ((height/8.0)/height)*normalizedLength;

        }

        unsigned int width, height;

        void createWindow(){
            glfwInit();            

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            GLFWwindow* window = glfwCreateWindow(this->width, this->height, "Chess", NULL, NULL);

            glfwMakeContextCurrent(window);

            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

            loadGlad();
            
            Shader ourShader("shaders/shader.vs", "shaders/shader.fs");

            glViewport(0, 0, this->width, this->height);
            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

            float board[boardSize];
            unsigned int whiteIndices[indicesSize/2];
            unsigned int blackIndices[indicesSize/2];
            
            createBackgroundTiles(board, whiteIndices, blackIndices);

            //We need to generalize this.
            float bigSquare[20] = {
                -1.0f+increment, -1.0f+increment, 0.0f, 1.0f, 1.0f, //bottomleft
                -1.0f+2*increment, -1.0f+increment, 0.0f, 0.0f, 1.0f, //bottomright
                -1.0f+2*increment, -1.0f+2*increment, 0.0f, 0.0f, 0.0f, //topright
                -1.0f+increment, -1.0f+2*increment, 0.0f, 1.0f, 0.0f //topleft
            };

            unsigned int bigSquareIndex[6] = {
                0, 1, 2,
                2, 3, 0
            };

            Texture whitePawn = Texture("resources/pawn_white.png");

            unsigned int VBO, EBO, VAO, VBO2, VAO2, EBO2;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
            glGenBuffers(1, &VBO2);
            glGenVertexArrays(1, &VAO2);
            glGenBuffers(1, &EBO2);

            glBindVertexArray(VAO);            

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(board), board, GL_STATIC_DRAW);


            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(whiteIndices), whiteIndices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(VAO2);
            glBindBuffer(GL_ARRAY_BUFFER, VBO2);
            glBufferData(GL_ARRAY_BUFFER, sizeof(bigSquare), bigSquare, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bigSquareIndex), bigSquareIndex, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) (3*sizeof(float)));
            glEnableVertexAttribArray(1);

            //Renders in wireframe mode
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //Let's find a better way to do what we're doing below...
            double prev_x = NULL;
            double prev_y = NULL;

            Input mouseInput = Input(window, width, height);

            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
            while(!glfwWindowShouldClose(window)){
                if(glfwGetKey(window, GLFW_KEY_ESCAPE) == 1){
                    glfwSetWindowShouldClose(window, true);
                }

                if (mouseInput.draggable){
                    //Here we want to move our object, we have pressed but haven't released
                    double current_x, current_y;                    
                    glfwGetCursorPos(window, &current_x, &current_y);
                    if (prev_x != NULL && prev_y != NULL){
                        if (current_x < width && current_y < height){
                            dragPiece(bigSquare, math::normalize(current_x - prev_x, width), math::normalize(current_y-prev_y, width));
                        }
                    }
                    else{
                        dragPiece(bigSquare, math::normalize(current_x-mouseInput.x, width), math::normalize(current_y-mouseInput.y, width));
                    }
                    prev_x = current_x;
                    prev_y = current_y;
                }
                else{
                    //Button released
                    prev_x = NULL;
                    prev_y = NULL;
                }

                glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                ourShader.use();

                glUniform1i(glGetUniformLocation(ourShader.ID, "isTextureBound"), 0);

                //This code needs to be more general and in functions.
                glBindVertexArray(VAO);   
                glUniform3f(glGetUniformLocation(ourShader.ID, "ourColor"), 1.0f, 1.0f, 1.0f);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(board), board, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(whiteIndices), whiteIndices, GL_STATIC_DRAW);         
                glDrawElements(GL_TRIANGLES, indicesSize/2, GL_UNSIGNED_INT, 0);

                glUniform3f(glGetUniformLocation(ourShader.ID, "ourColor"), 0.0f, 0.0f, 0.0f);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(blackIndices), blackIndices, GL_STATIC_DRAW);

                glDrawElements(GL_TRIANGLES, indicesSize/2, GL_UNSIGNED_INT, 0);

                glBindVertexArray(VAO2);

                glBindTexture(GL_TEXTURE_2D, whitePawn.texture);

                glBindBuffer(GL_ARRAY_BUFFER, VBO2);
                glBufferData(GL_ARRAY_BUFFER, sizeof(bigSquare), bigSquare, GL_STATIC_DRAW);  
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bigSquareIndex), bigSquareIndex, GL_STATIC_DRAW);

                glUniform3f(glGetUniformLocation(ourShader.ID, "ourColor"), 0.2f, 0.5f, 0.7f);

                glUniform1i(glGetUniformLocation(ourShader.ID, "isTextureBound"), 1);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glfwSwapBuffers(window);
                glfwPollEvents();
            }

            glfwTerminate();
            return;

        }
    private:
        void loadGlad(){
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                std::cout << "Failed to initialize GLAD" << std::endl;
            }
        }

        void createBackgroundTiles(float * boardArray, unsigned int * whiteIndices, unsigned int * blackIndices){
            for (int i = 0; i<9; i++){
                for (int j = 0; j<9; j++){
                    int rowShift = i*(columns+1)*VERTEX_SIZE;
                    int colShift = VERTEX_SIZE*j;
                    int positionIncrement = rowShift+colShift;
                    Vector2 vector;
                    vector.x = START_X+j*this->increment;
                    vector.y = START_Y+i*this->increment;
                    *(boardArray + positionIncrement) = vector.x; //pos 1
                    *(boardArray + positionIncrement + 1) = vector.y; //pos 2
                    *(boardArray + positionIncrement + 2) = 0; //pos 3, always 0.
                }
            }

            int blackCounter = 0;
            int whiteCounter = 0;
            bool black = 1;
            bool blackEven = 1;
            //This is impossible to read.
            for (int i = 0; i<8; i++){
                if (i % 2 == 0){
                    blackEven = 1;
                } else{blackEven = 0;}
                for (int j = 0; j<8; j++){
                    if (j%2==0){
                        if (blackEven){
                            black = 1;
                        }
                        else{
                            black = 0;
                        }
                    }
                    else{
                        if (blackEven){
                            black = 0;
                        }
                        else{
                            black = 1;
                        }
                    }
                    if (black){
                        insertIntoIndices(blackIndices, i, j, &blackCounter);
                    }
                    else{
                        insertIntoIndices(whiteIndices, i, j, &whiteCounter);
                    }
                }
            }
        }

        void insertIntoIndices(unsigned int * indices, int i, int j, int * counter){
            int firstPos = i*(rows+1)+j;
            int secondPos = (i+1)*(rows+1)+j;
            int thirdPos = i*(rows+1)+j+1;
            *(indices+(*counter)++) = firstPos;
            *(indices+(*counter)++) = secondPos;
            *(indices+(*counter)++) = thirdPos;
            //This is the fourth vertex to draw a square, placed one to the right and one up.
            //Hence taking the third position (one to the right) and adding 9 (one up).
            *(indices+(*counter)++) = thirdPos+9;
            *(indices+(*counter)++) = secondPos;
            *(indices+(*counter)++) = thirdPos;
        }

        void dragPiece(float * piece, float dx, float dy){
            bool stop_x = 0;
            bool stop_y = 0;

            //You can reach different edges of the screen:
            //NB: Kæmpe problem her!

            if ((piece[0] + dx > 1 || piece[0] + dx < -1) || (piece[5] + dx > 1 || piece[5] + dx < -1)){
                stop_x = 1;
            }
            if ((piece[1] + dy > 1 || piece[1] + dy < -1) || (piece[11] + dy > 1 || piece[11] + dy < -1)){
                stop_y = 1;
            }

            std::cout << "Dragging with: " << dx << " and " << dy;
            for (int i = 0; i<20; i+=5){
                //To the left this works, but not to the right because we start with points that pass the check.
                if (piece[i] + dx <= 1 && piece[i] + dx >=-1){
                    if (!stop_x){
                        piece[i] += dx;
                    }
                }
                if (piece[i+1] + dy <= 1 && piece[i+1] + dy >=-1){
                    if (!stop_y){
                        piece[i+1] += -dy;
                    }
                }
            }
        }   
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Input::width = width;
    Input::height = height;
    glViewport(0, 0, width, height);
}

int main(){

    Engine eng(800, 800);
    eng.createWindow();
    std::cout << "Program has stopped running.";
    return 0;

}
