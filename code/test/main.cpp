#include "animation.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model.h"
#include "object3d.h"
#include "field.h"
#include "camera.h"

#include <vector>
#include "application.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
using Vec3 = glm::vec3;

class MassSpringAnimation : public PhysicsAnimation, public Application
{
public:
    MassSpringAnimation(int num = 5) : PhysicsAnimation(), Application()
    {
        frame.reset(new Frame(0, 1.0 / 60));
        numberOfPoints = num;
        // set parameter
        setParameter();
        makeChain(10);

        // render about
        _windowTitle = "Mass Spring Animation";

        Plane floor(Vec3(0, 0, -10), Vec3(0, 0, 1));

        sphere.reset(new Model("../data/sphere.obj"));
        sphere->scale = Vec3(0.1, 0.1, 0.1);

        camera.reset(new Camera(glm::vec3(0, 0, 10)));

        modelMatrices.resize(numberOfPoints);

        /*std::string vsfile = "../test/Instanced.vs";
        std::string floorvs = "../test/floor.vs";
        std::string fsfile = "../test/Instanced.fs";*/
        std::string vsfile = "C:/Users/Stellaris/Desktop/ParticleBasedFluid/code/test/Instanced.vs";
        std::string floorvs = "C:/Users/Stellaris/Desktop/ParticleBasedFluid/code/test/floor.vs";
        std::string fsfile = "C:/Users/Stellaris/Desktop/ParticleBasedFluid/code/test/Instanced.fs";
        floorShader.reset(new Shader(floorvs, fsfile));
        sphereShader.reset(new Shader(vsfile, fsfile));

        // init imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(_window, true);
        ImGui_ImplOpenGL3_Init();
    }

    ~MassSpringAnimation()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    /* derived class can override this function to handle input */
    virtual void handleInput()
    {
        onAdvanceTimeStep(_deltaTime);
        if (_keyboardInput.keyStates[GLFW_KEY_W] != GLFW_RELEASE)
        {
            camera->ProcessKeyboard(FORWARD, _deltaTime);
        }

        if (_keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE)
        {
            camera->ProcessKeyboard(LEFT, _deltaTime);
        }

        if (_keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE)
        {
            camera->ProcessKeyboard(BACKWARD, _deltaTime);
        }

        if (_keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE)
        {
            camera->ProcessKeyboard(RIGHT, _deltaTime);
        }
    }

    /* derived class can override this function to render a frame */
    virtual void renderFrame()
    {
        // update(*frame);
        // frame->advance();

        for (int i = 0; i < numberOfPoints; i++)
        {
            glm::mat4 model = sphere->getModelMatrix();
            modelMatrices[i] = glm::translate(model, positions[i]);
        }
        showFpsInWindowTitle();

        glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // get camera properties
        const glm::mat4 &projection = glm::perspective(glm::radians(camera->Zoom), (float)_windowWidth / (float)_windowHeight, 0.1f, 100.0f);
        const glm::mat4 &view = camera->GetViewMatrix();

        floorShader->use();
        floorShader->setMat4("view", view);
        floorShader->setMat4("projection", projection);
        floorShader->setMat4("model", glm::mat4(1.0f));
        floor.draw();

        sphereShader->use();

        sphereShader->setMat4("view", view);
        sphereShader->setMat4("projection", projection);
        glBindVertexArray(sphere->_vao);

        unsigned int instanceVBO;
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numberOfPoints, &modelMatrices[0], GL_STATIC_DRAW);

        GLsizei vec4Size = sizeof(glm::vec4);
        printf("%d\n", vec4Size);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(3 * vec4Size));
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
        sphere->instancedDraw(numberOfPoints);
    }

private:
    void onAdvanceTimeStep(float timeInterval) override
    {
        for (int i = 0; i < positions.size(); i++)
        {
            // Gravity
            forces[i] = gravity * float(mass);
            // Air drag
            Vec3 relativeVel = velocities[i];
            if (wind != nullptr)
            {
                relativeVel -= wind->sample(positions[i]);
            }
            forces[i] += -dragCoefficient * relativeVel;
        }

        for (int i = 0; i < edges.size(); i++)
        {
            int pid0 = edges[i].first, pid1 = edges[i].second;

            // spring force
            Vec3 pos0 = positions[pid0], pos1 = positions[pid1];
            Vec3 r = pos0 - pos1;
            float distance = glm::length(r);
            if (distance > 0)
            {
                Vec3 force = -stiffness * (distance - restLength) * glm::normalize(r);
                forces[pid0] += force;
                forces[pid1] -= force;
            }
            // damping force
            Vec3 v0 = velocities[pid0], v1 = velocities[pid1];
            Vec3 relativeV = v0 - v1;
            Vec3 damping = -dampingCoefficient * relativeV;
            forces[pid0] += damping;
            forces[pid1] -= damping;
        }
        // Update states
        for (int i = 0; i < positions.size(); ++i)
        {
            // Compute new states
            Vec3 newAcceleration = forces[i] * (1.0f / mass);
            Vec3 newVelocity = velocities[i] + timeInterval * newAcceleration;
            Vec3 newPosition = positions[i] + timeInterval * newVelocity;

            // Collision
            if (newPosition.y < floorPositionY)
            {
                newPosition.y = floorPositionY;

                if (newVelocity.y < 0.0)
                {
                    newVelocity.y *= -restitutionCoefficient;
                    newPosition.y += timeInterval * newVelocity.y;
                }
            }

            // Update states
            velocities[i] = newVelocity;
            positions[i] = newPosition;
        }

        // Apply constraints
        for (int i = 0; i < constraints.size(); ++i)
        {
            size_t pointIndex = constraints[i].pointIndex;
            positions[pointIndex] = constraints[i].fixedPosition;
            velocities[pointIndex] = constraints[i].fixedVelocity;
        }
    }
    void setParameter()
    {
        mass = 1.0;
        gravity = Vec3(0.0, -9.8, 0.0);
        stiffness = 500.0;
        restLength = 2.0;
        dampingCoefficient = 1.0;
        dragCoefficient = 0.1;

        floorPositionY = -7.0;
        restitutionCoefficient = 0.3;

        constraints.push_back(Constraint{0, Vec3(0), Vec3(0)});
        wind = std::make_shared<ConstantVectorField>(Vec3(30.0, 0, 0));
    }
    void makeChain(int numberOfPoints)
    {
        if (numberOfPoints == 0)
        {
            return;
        }

        int numberOfEdges = numberOfPoints - 1;

        positions.resize(numberOfPoints);
        velocities.resize(numberOfPoints);
        forces.resize(numberOfPoints);
        edges.resize(numberOfEdges);

        for (int i = 0; i < numberOfPoints; ++i)
        {
            positions[i].x = -static_cast<float>(i);
        }

        for (int i = 0; i < numberOfEdges; ++i)
        {
            edges[i] = Edge{i, i + 1};
        }
    }
    struct Edge
    {
        int first;
        int second;
    };

    struct Constraint
    {
        int pointIndex;
        Vec3 fixedPosition;
        Vec3 fixedVelocity;
    };

    std::unique_ptr<Frame> frame;

    int numberOfPoints;
    float mass;
    Vec3 gravity;
    float stiffness;
    float restLength;
    float dampingCoefficient;
    float dragCoefficient;

    float floorPositionY;
    float restitutionCoefficient;

    std::vector<Vec3> positions;
    std::vector<Vec3> velocities;
    std::vector<Vec3> forces;
    std::vector<Edge> edges;

    std::shared_ptr<VectorField> wind;
    std::vector<Constraint> constraints;

    Plane floor;
    std::unique_ptr<Model> sphere;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Shader> sphereShader;
    std::unique_ptr<Shader> floorShader;
    std::vector<glm::mat4> modelMatrices;
};

int main()
{
    try
    {
        MassSpringAnimation app(10);
        app.run();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}