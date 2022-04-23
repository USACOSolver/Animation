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
        makeChain();

        // render about
        _windowTitle = "Mass Spring Animation";

        floor.reset(new Plane(Vec3(0, floorPositionY - 1, 0), Vec3(0, 1, 0)));

        sphere.reset(new Model("../data/sphere.obj"));
        sphere->scale = Vec3(1, 1, 1);

        camera.reset(new Camera(glm::vec3(0, 0, 30)));

        modelMatrices.resize(numberOfPoints);

        std::string vsfile = "../test/Instanced.vs";
        std::string floorvs = "../test/floor.vs";
        std::string fsfile = "../test/Instanced.fs";
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
        float d = 50 * SPEED * _deltaTime;
        if (_keyboardInput.keyStates[GLFW_KEY_W] != GLFW_RELEASE)
        {
            camera->ProcessMouseMovement(0, d);
        }

        if (_keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE)
        {
            camera->ProcessMouseMovement(-d, 0);
        }

        if (_keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE)
        {
            camera->ProcessMouseMovement(0, -d);
        }

        if (_keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE)
        {
            camera->ProcessMouseMovement(d, 0);
        }
        if (_keyboardInput.keyStates[GLFW_KEY_Q] != GLFW_RELEASE)
        {
            camera->ProcessMouseScroll(d);
        }
        if (_keyboardInput.keyStates[GLFW_KEY_E] != GLFW_RELEASE)
        {
            camera->ProcessMouseScroll(-d);
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

        glm::vec3 lightdir(1, -1, 1);
        lightdir = -glm::normalize(lightdir);

        floorShader->use();
        floorShader->setMat4("view", view);
        floorShader->setMat4("projection", projection);
        floorShader->setVec3("lightDir", lightdir);
        floorShader->setMat4("model", glm::mat4(1.0f));
        floorShader->setVec4("color", glm::vec4(0.6, 0.6, 0.6, 1));

        floor->draw();

        sphereShader->use();

        sphereShader->setMat4("view", view);
        sphereShader->setMat4("projection", projection);
        sphereShader->setVec4("color", glm::vec4(0.5, 0, 0, 1));
        sphereShader->setVec3("lightDir", lightdir);
        glBindVertexArray(sphere->_vao);

        unsigned int instanceVBO;
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numberOfPoints, &modelMatrices[0], GL_STATIC_DRAW);

        GLsizei vec4Size = sizeof(glm::vec4);
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

        // imgui
        // draw ui elements
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const auto flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings;
        bool control_panel = true;
        if (!ImGui::Begin("Control Panel", &control_panel, flags))
        {
            ImGui::End();
        }
        else
        {
            int number = numberOfPoints;
            ImGui::SliderInt("Number of balls", &number, 1, 10);
            float g = 9.8;
            ImGui::SliderFloat("Gravity", &g, 0, 20);
            float rl = restLength;
            ImGui::SliderFloat("Rest Length", &rl, 0, 5);
            if (ImGui::Button("Restart!"))
            {
                restLength = rl;
                numberOfPoints = number;
                gravity.y = -g;

                makeChain();
            }
            ImGui::End();
        }
        bool camera_window = true;
        if (!ImGui::Begin("Camera Parameter", &camera_window, flags))
        {
            ImGui::End();
        }
        else
        {
            glm::vec3 pos = camera->Position;
            std::string text = "Camera Position X: " + std::to_string(pos.x);
            ImGui::Text(text.c_str());
            ImGui::NewLine();
            text = "Camera Position Y: " + std::to_string(pos.y);
            ImGui::Text(text.c_str());
            ImGui::NewLine();
            text = "Camera Position Z: " + std::to_string(pos.z);
            ImGui::Text(text.c_str());
            ImGui::End();
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

        floorPositionY = -10.0;
        restitutionCoefficient = 0.3;

        constraints.push_back(Constraint{0, Vec3(0), Vec3(0)});
        wind = std::make_shared<ConstantVectorField>(Vec3(30.0, 0, 0));
    }
    void makeChain()
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
            positions[i] = glm::vec3(-static_cast<float>(i), 0, 0);
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

    std::unique_ptr<Plane> floor;
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