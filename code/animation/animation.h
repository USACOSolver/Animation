#ifndef _ANIMATION_H_
#define _ANIMATION_H_

class Frame
{
public:
    Frame(){};
    Frame(int newIndex, float newTimeInterval) : index(newIndex), timeInterval(newTimeInterval) {}
    ~Frame(){};
    void advance() { index++; }
    void advance(unsigned int delta)
    {
        index += delta;
    }

    int index = 0;
    float timeInterval = 1.0f / 60.0f;
};

class Animation
{
public:
    void update(const Frame &frame)
    {
        // pre-processing
        onUpdate(frame);
        // post-processing
    }

protected:
    virtual void onUpdate(const Frame &frame) = 0;
};

class PhysicsAnimation : public Animation
{

protected:
    virtual void onUpdate(const Frame &frame) override
    {
        if (frame.index > _currentFrame.index)
        {
            unsigned int n = frame.index - _currentFrame.index;

            for (int i = 0; i < n; i++)
            {
                advanceTimeStep(frame.timeInterval);
            }
            _currentFrame = frame;
        }
    }
    virtual void onAdvanceTimeStep(float timeInterval) = 0;

private:
    Frame _currentFrame;
    void advanceTimeStep(float timeInterval){};
};
#endif