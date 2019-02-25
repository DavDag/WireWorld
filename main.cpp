#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>

float zoom = 1.0f;

class WireWorld: public sf::Drawable, public sf::Transformable
{
    public:
        enum CellType : unsigned char {Empty, ElectronHead, ElectronTail, Conductor};

    public:
        WireWorld(int rows, int columns, int width, int height):
            m_World((rows + 1) * (columns + 1), WireWorld::CellType::Empty),
            m_WorldBuffer(m_World.size()),
            clock(),
            rows(rows),
            columns(columns),
            width(width),
            height(height),
            pause(false),
            hidden(false),
            FrameRate(1 / 15.0f)
        {
            srand(time(NULL));
        }

        void trigPause() {pause = !pause;}
        void trigHidden() {hidden= !hidden;}

        void setCell(WireWorld::CellType type, sf::Vector2f pos)
        {
            for(int i = 1; i < rows; ++i) for(int j = 1; j < columns; ++j)
            {
                if((pos.y >= i * height) && (pos.y < (i + 1) * height) && (pos.x >= (j) * width) && (pos.x < (j + 1) * width))
                {
                    m_World[i * columns + j] = type;
                    break;
                }
            }
        }

        void update()
        {
            if(pause) return;
            if(clock.getElapsedTime().asSeconds() < FrameRate) return;
            clock.restart();
            std::copy(m_World.begin(), m_World.end(), m_WorldBuffer.begin());
            for(int i = 1; i < rows; ++i) for(int j = 1; j < columns; ++j)
            {
                if(m_WorldBuffer[i * columns + j] == WireWorld::CellType::Empty) continue;
                else if(m_WorldBuffer[i * columns + j] == WireWorld::CellType::ElectronHead)
                {
                    m_World[i * columns + j] = WireWorld::CellType::ElectronTail;
                    continue;
                }
                else if(m_WorldBuffer[i * columns + j] == WireWorld::CellType::ElectronTail)
                {
                    m_World[i * columns + j] = WireWorld::CellType::Conductor;
                    continue;
                }
                unsigned char cnt = 0;
                if(m_WorldBuffer[(i + 1) * columns + (j + 1)]   == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i + 1) * columns + (j - 1)]   == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i + 1) * columns + (j)]       == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i - 1) * columns + (j + 1)]   == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i - 1) * columns + (j - 1)]   == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i - 1) * columns + (j)]       == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i) * columns + (j + 1)]       == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i) * columns + (j - 1)]       == WireWorld::CellType::ElectronHead) ++cnt;
                if(m_WorldBuffer[(i) * columns + (j)]           == WireWorld::CellType::ElectronHead) ++cnt;
                if(cnt == 1 || cnt == 2) m_World[i * columns + j] = WireWorld::CellType::ElectronHead;
            }
        }

        void draw(sf::RenderTarget &rt, sf::RenderStates rs) const
        {
            rs.texture = NULL;
            rs.transform *= getTransform();
            sf::RectangleShape rect(sf::Vector2f(width * (columns - 1), height * (rows - 1)));
            rect.setPosition(width, height);
            rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineColor(sf::Color(128, 128, 128));
            rect.setOutlineThickness(zoom);
            rt.draw(rect, rs);
            rect.setSize(sf::Vector2f(width, height));
            rect.setOutlineThickness((zoom > 2.0f) ? 0 : -1);
            for(int i = 1; i < rows; ++i) for(int j = 1; j < columns; ++j)
            {
                if(m_World[i * columns + j]         == WireWorld::CellType::Empty)          rect.setFillColor(sf::Color::Black);
                else if(m_World[i * columns + j]    == WireWorld::CellType::ElectronHead)   rect.setFillColor(sf::Color::Blue);
                else if(m_World[i * columns + j]    == WireWorld::CellType::ElectronTail)   rect.setFillColor(sf::Color::Red);
                else if(m_World[i * columns + j]    == WireWorld::CellType::Conductor && hidden)   rect.setFillColor(sf::Color::Black);
                else if(m_World[i * columns + j]    == WireWorld::CellType::Conductor)   rect.setFillColor(sf::Color::Yellow);
                rect.setPosition(j * width, i * height);
                rt.draw(rect, rs);
            }
        }

    private:
        std::vector<unsigned char> m_World, m_WorldBuffer;
        sf::Clock clock;
        int rows, columns, width, height;
        bool pause, hidden;
        float FrameRate;
};

int main()
{
    sf::RenderWindow app(sf::VideoMode(800, 600), "WireWorld");
    app.setFramerateLimit(60);
    sf::View view(sf::Vector2f(400, 300), sf::Vector2f(800, 600));
    sf::Vector2i mousePos;
    WireWorld ww(64, 64, 64, 64);
    bool drag = false;
    while (app.isOpen())
    {
        if(!app.hasFocus()) continue;
        sf::Event event;
        while(app.pollEvent(event))
        {
            if(event.type == sf::Event::Closed) app.close();
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) ww.trigPause();
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H) ww.trigHidden();
            else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle)
            {
                drag = true;
                mousePos = sf::Mouse::getPosition(app);
            }
            else if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Middle) drag = false;
            else if(event.type == sf::Event::MouseMoved && drag)
            {
                sf::Vector2i tmpMousePos = sf::Mouse::getPosition(app);
                view.move(sf::Vector2f(mousePos - tmpMousePos) * zoom);
                mousePos = tmpMousePos;
            }
            else if(event.type == sf::Event::MouseWheelScrolled)
            {
                view.zoom(1 / zoom);
                zoom *= (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                view.zoom(zoom);
            }
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            ww.setCell(WireWorld::CellType::Conductor, app.mapPixelToCoords(sf::Mouse::getPosition(app)));
        else if(false && sf::Mouse::isButtonPressed(sf::Mouse::Middle))
            ww.setCell(WireWorld::CellType::ElectronHead, app.mapPixelToCoords(sf::Mouse::getPosition(app)));
        else if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
            ww.setCell(WireWorld::CellType::Empty, app.mapPixelToCoords(sf::Mouse::getPosition(app)));
        ww.update();
        app.clear();
//        app.setView(app.getDefaultView());
//        app.drawGui();
        app.setView(view);
        app.draw(ww);
        app.display();
    }
    return EXIT_SUCCESS;
}
