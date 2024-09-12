#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

class body {
public:
    double mass;
    double xpos;
    double ypos;
    double xvel;
    double yvel;

    sf::CircleShape s;
    sf::VertexArray trail;
    sf::Color color;

    double next_xvel;
    double next_yvel;

    body () {}
    body(double m, double posx, double posy, double velx, double vely, sf::Color c)
    {
        mass = m;
        xpos = posx;
        ypos = posy;
        xvel = velx;
        yvel = vely;
        color = c;

        next_xvel = 0;
        next_yvel = 0;

        s.setRadius(m+1);
        s.setPosition(xpos - s.getRadius(), ypos - s.getRadius());
        s.setFillColor(color);

        trail = sf::VertexArray::VertexArray(sf::LinesStrip, 1000);
        for (int i = 0; i < trail.getVertexCount(); i++)
        {
            trail[i].position = sf::Vector2f(posx, posy);
            trail[i].color = color;
        }
    }

    void calcTrajectory(std::vector<body*> bodies, double timestep)
    {
        double ax = 0;
        double ay = 0;

        for (int i = 0; i < bodies.size(); i++)
        {
            body* body = bodies[i];

            if (body == this)
            {
                continue;
            }

            double rpx = body->xpos - xpos;
            double rpy = body->ypos - ypos;

            double a1x = (body->mass) / (rpx * rpx + rpy * rpy) * rpx;
            double a1y = (body->mass) / (rpx * rpx + rpy * rpy) * rpy;

            ax += a1x;
            ay += a1y;
        }

        next_xvel = xvel + ax * timestep;
        next_yvel = yvel + ay * timestep;
    }

    void update(double timestep)
    {
        xpos = xpos + xvel * timestep;
        ypos = ypos + yvel * timestep;
        xvel = next_xvel;
        yvel = next_yvel;
        s.setPosition(xpos - s.getRadius(), ypos - s.getRadius());

        for (int i = trail.getVertexCount() - 1; i > 0; i--)
        {
            trail[i]= trail[i - 1];
        }

        trail[0].position = sf::Vector2f(xpos, ypos);
        trail[0].color = color;
    }

    void render(sf::RenderWindow& wind, bool showTrail)
    {
        wind.draw(s);

        if (showTrail)
        {
            wind.draw(trail);
        }
    }

    void print()
    {
        std::cout << "Position[" << xpos << ", " << ypos << "], Velocity[" << xvel << ", " << yvel << "]" << std::endl;
    }

    float getEnergy(std::vector<body*> bodies)
    {
        float ke = .5 * mass * (pow(xvel, 2) + pow(yvel,2));
        float pe = 0;

        for (int i = 0; i < bodies.size(); i++)
        {
            body* body = bodies[i];
            if (body == this)
            {
                continue;
            }

            double rpx = body->xpos - xpos;
            double rpy = body->ypos - ypos;

            pe -= .5 * mass * body->mass / sqrt((pow(rpx,2) + pow(rpy,2)));
        }

        return ke + pe;
    }
};

class moon : public body {
public:
    moon() {}
    moon(double posx, double posy, double velx, double vely, sf::Color c)
    {
        xpos = posx;
        ypos = posy;
        xvel = velx;
        yvel = vely;
        color = c;

        next_xvel = 0;
        next_yvel = 0;

        s.setRadius(1);
        s.setPosition(xpos - s.getRadius(), ypos - s.getRadius());
        s.setFillColor(color);

        trail = sf::VertexArray::VertexArray(sf::LinesStrip, 1000);
        for (int i = 0; i < trail.getVertexCount(); i++)
        {
            trail[i].position = sf::Vector2f(posx, posy);
            trail[i].color = color;
        }
    }
};

sf::CircleShape getCOG(std::vector<body*> bodies)
{
    float totalMass = 0;
    sf::Vector2f COG;

    for (int i = 0; i < bodies.size(); i++)
    {
        totalMass += bodies[i]->mass;
        COG.x += bodies[i]->mass * bodies[i]->xpos;
        COG.y += bodies[i]->mass * bodies[i]->ypos;
    }

    COG = COG / totalMass;

    sf::CircleShape s;
    s.setRadius(3);
    s.setPosition(COG);
    s.setFillColor(sf::Color::Cyan);

    return s;
}

//Code fom SFML forums
sf::Color hsv(int hue, float sat, float val)
{
    hue %= 360;
    while (hue < 0) hue += 360;

    if (sat < 0.f) sat = 0.f;
    if (sat > 1.f) sat = 1.f;

    if (val < 0.f) val = 0.f;
    if (val > 1.f) val = 1.f;

    int h = hue / 60;
    float f = float(hue) / 60 - h;
    float p = val * (1.f - sat);
    float q = val * (1.f - sat * f);
    float t = val * (1.f - sat * (1 - f));

    switch (h)
    {
    default:
    case 0:
    case 6: return sf::Color(val * 255, t * 255, p * 255);
    case 1: return sf::Color(q * 255, val * 255, p * 255);
    case 2: return sf::Color(p * 255, val * 255, t * 255);
    case 3: return sf::Color(p * 255, q * 255, val * 255);
    case 4: return sf::Color(t * 255, p * 255, val * 255);
    case 5: return sf::Color(val * 255, p * 255, q * 255);
    }
}

void renderLaunch(sf::RenderWindow& window, sf::Vector2i launchPosition, double mass, sf::Color launchColor)
{
    sf::CircleShape s;
    sf::Vector2i position = sf::Mouse::getPosition(window);

    sf::VertexArray launchLine(sf::Lines, 2);
    launchLine[0] = sf::Vector2f(position);
    launchLine[0].color = sf::Color::Red;

    launchLine[1] = sf::Vector2f(launchPosition);
    launchLine[1].color = sf::Color::Red;

    s.setRadius(mass+1);
    s.setPosition((float)position.x - s.getRadius(), (float)position.y - s.getRadius());
    s.setFillColor(launchColor);
    window.draw(s);
    window.draw(launchLine);
}

int main(int argc, char* argv[])
{
    double timestep = .1f;
    int width = 1920;
    int height = 1080;

    sf::RenderWindow window(sf::VideoMode(width, height), "Hello Worlds!");
    window.setFramerateLimit(240);

    std::vector<body*> bodies;
    std::vector<moon*> moons;

    if (argv[1] == "--figure8")
    {
        double mass = 5;
        double init_xv = -1.1;
        double init_yv = -.85;
        double mag = 1.04;

        body* body1 = new body(mass, (double)(width / 2 - 200), (double)(height / 2), mag * init_xv, mag * init_yv, sf::Color::White);
        body* body2 = new body(mass, (double)(width / 2), (double)(height / 2), -2 * mag * init_xv, -2 * mag * init_yv, sf::Color::White);
        body* body3 = new body(mass, (double)(width / 2 + 200), (double)(height / 2), mag * init_xv, mag * init_yv, sf::Color::White);

        bodies.push_back(body1);
        bodies.push_back(body2);
        bodies.push_back(body3);

        const int numPlanets = 60;
        for (int i = 0; i < numPlanets; i++)
        {
            sf::Color color = hsv((360/numPlanets)*i, 1, 1);
            moon* thisMoon = new moon(100 + (width / 2) + .01 * i, 100 + (height / 2) + .01 * i, 0, 0, color);
            moons.push_back(thisMoon);
        }
    }

    bool showInfo = false;
    bool showTrails = true;
    bool followCOG = false;
    bool paused = false;
    int colorIndex = 0;

    bool isLaunching = false;
    double launchMass = 5;
    sf::Color launchColor;
    sf::Vector2i launchPosition;
    sf::Vector2f launchVelocity;


    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                launchPosition = sf::Mouse::getPosition(window);
                launchColor = hsv(rand() % 360, 1, 1);
                isLaunching = true;
            }

            if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && isLaunching == true)
            {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                launchVelocity.x = ((float)launchPosition.x - (float)mousePosition.x)/20;
                launchVelocity.y = ((float)launchPosition.y - (float)mousePosition.y)/20;
                body* newBody = new body(launchMass, mousePosition.x, mousePosition.y, launchVelocity.x, launchVelocity.y, launchColor);
                bodies.push_back(newBody);
                isLaunching = false;
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
            {
                if (showInfo == false) { showInfo = true; }
                else { showInfo = false; }
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T)
            {
                if (showTrails == false) { showTrails = true; }
                else { showTrails = false; }
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
            {
                if (paused == false) { paused = true; }
                else { paused = false; }
            }

            if (event.type == sf::Event::MouseWheelMoved && isLaunching == true)
            {
                launchMass = std::max(1.0, launchMass+event.mouseWheel.delta);
            }

            if (event.type == sf::Event::MouseWheelMoved && isLaunching == false)
            {
                timestep = timestep * pow(.9, event.mouseWheel.delta);
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C)
            {
                if (followCOG == false) { followCOG = true; }
                else { followCOG = false; }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
            {
                sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

                int numMoons = 30;
                for (int i = 0; i < numMoons; i++)
                {
                    sf::Color color = hsv((360 / numMoons) * i, 1, 1);
                    moon* thisMoon = new moon(mousePos.x + .01 * i, mousePos.y + .01 * i, .01, .01, color);
                    moons.push_back(thisMoon);
                }

            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                bodies.clear();
                moons.clear();
            }
        }

        window.clear();

        for (int i = 0; i < bodies.size(); i++)
        {
            body* body = bodies[i];
            body->render(window, showTrails);
            body->calcTrajectory(bodies, timestep);
        }

        for (int i = 0; i < moons.size(); i++)
        {
            moon* body = moons[i];
            body->render(window, showTrails);
            body->calcTrajectory(bodies, timestep);
        }

        if (!paused)
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                body* body = bodies[i];
                body->update(timestep);
            }

            for (int i = 0; i < moons.size(); i++)
            {
                moon* body = moons[i];
                body->update(timestep);
            }
        }

        if (showInfo) 
        { window.draw(getCOG(bodies)); }

        if (isLaunching) { renderLaunch(window, launchPosition, launchMass, launchColor); }

        if (followCOG)
        {
            sf::Vector2f cog = getCOG(bodies).getPosition();
            for (int i = 0; i < bodies.size(); i++)
            {
                bodies[i]->xpos -= cog.x - width / 2;
                bodies[i]->ypos -= cog.y - height / 2;
            }
        }  

        window.display();
    }

    return 0;
}