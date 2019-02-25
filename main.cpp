#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>

float zoom = 1.0f;
sf::Texture trash_can;

class WireWorld: public sf::Drawable, public sf::Transformable
{
    public:
        enum CellType : unsigned char {Empty, Conductor, ElectronHead, ElectronTail};

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
        ~WireWorld() {};

        void trigPause() {pause = !pause;}
        void trigHidden() {hidden= !hidden;}
        void fillAll(WireWorld::CellType type) {std::fill(m_World.begin(), m_World.end(), type);}

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
                if(m_World[i * columns + j]         == WireWorld::CellType::Empty)          rect.setFillColor(sf::Color::Transparent);
                else if(m_World[i * columns + j]    == WireWorld::CellType::ElectronHead)   rect.setFillColor(sf::Color::Blue);
                else if(m_World[i * columns + j]    == WireWorld::CellType::ElectronTail)   rect.setFillColor(sf::Color::Red);
                else if(m_World[i * columns + j]    == WireWorld::CellType::Conductor && hidden)   rect.setFillColor(sf::Color::Transparent);
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

class Gui: public sf::Drawable, public sf::Transformable
{
    public:
        Gui(sf::Vector2f windSize, sf::Vector2f viewSize):
            hover(-1),
            option(WireWorld::CellType::Conductor),
            buttons(0),
            bg(sf::Vector2f(windSize.x - viewSize.x, windSize.y)),
            s()
        {
            for(int i = 0; i < 4; ++i)
            {
                buttons.push_back(sf::RectangleShape(sf::Vector2f(50, 50)));
                buttons[i].setPosition(737, i * 100 + 100);
                buttons[i].setOutlineThickness(4);
                buttons[i].setOutlineColor(sf::Color::Transparent);
            }
            buttons[0].setFillColor(sf::Color::White);
            buttons[1].setFillColor(sf::Color::Yellow);
            buttons[2].setFillColor(sf::Color::Blue);
            buttons[3].setFillColor(sf::Color::White);
            trash_can.loadFromFile("assets/trash_can.png");
            s.setTexture(trash_can);
            s.scale(50 / 64.0f, 50 / 64.0f);
            s.setPosition(buttons[3].getPosition());
            bg.setPosition(viewSize.x, 0);
            bg.setFillColor(sf::Color(64, 64, 64));
        }
        ~Gui() {}

        bool getHover() const {return (hover == -1);}
        void update() {}
        WireWorld::CellType getSelectedCellType() const {return option;}

        unsigned char setAction()
        {
            if(hover > 2) return hover - 2;
            option = (WireWorld::CellType)((hover == -1) ? 0 : (unsigned char)hover);
            return 0;
        }

        void checkHover(sf::Vector2f pos)
        {
            hover = -1;
            for(unsigned int i = 0; i < buttons.size(); ++i)
            {
                buttons[i].setOutlineColor(sf::Color::Transparent);
                if(buttons[i].getGlobalBounds().contains(pos))
                {
                    hover = i;
                    buttons[i].setOutlineColor(sf::Color::Red);
                    break;
                }
            }
        }

        void draw(sf::RenderTarget& rt, sf::RenderStates rs) const
        {
            rs.texture = NULL;
            rs.transform *= getTransform();
            rt.draw(bg, rs);
            for(unsigned int i = 0; i < buttons.size(); ++i)
                rt.draw(buttons[i], rs);
            rt.draw(s, rs);
        }
    private:
        int hover;
        WireWorld::CellType option;
        std::vector<sf::RectangleShape> buttons;
        sf::RectangleShape bg;
        sf::Sprite s;
};

int main()
{
    sf::RenderWindow app(sf::VideoMode(800, 600), "WireWorld");
    app.setFramerateLimit(60);
    sf::View view(sf::Vector2f(300, 300), sf::Vector2f(800 * 0.9f, 600));
    view.setViewport(sf::FloatRect(0, 0, 0.9f, 1.0f));
    sf::Vector2i mousePos;
    WireWorld ww(64, 64, 32, 32);
    Gui gui((sf::Vector2f)app.getSize(), view.getSize());
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
            else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                if(!gui.getHover())
                {
                    unsigned char action = gui.setAction();
                    if(action != 0) ww.fillAll((WireWorld::CellType)(action - 1));
                }
            }
            else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle)
            {
                drag = true;
                mousePos = sf::Mouse::getPosition(app);
            }
            else if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Middle) drag = false;
            else if(event.type == sf::Event::MouseMoved)
            {
                app.setView(app.getDefaultView());
                gui.checkHover(app.mapPixelToCoords(sf::Mouse::getPosition(app)));
                app.setView(view);
                if(!drag) continue;
                sf::Vector2i tmpMousePos = sf::Mouse::getPosition(app);
                view.move(sf::Vector2f(mousePos - tmpMousePos) * zoom);
                mousePos = tmpMousePos;
            }
            else if(event.type == sf::Event::MouseWheelScrolled)
            {
                view.zoom(1 / zoom);
                zoom *= (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                zoom = std::max(std::min(zoom, 10.0f), 0.1f);
                view.zoom(zoom);
            }
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && gui.getHover())
            ww.setCell(gui.getSelectedCellType(), app.mapPixelToCoords(sf::Mouse::getPosition(app)));
        gui.update();
        ww.update();
        app.clear(sf::Color::White);
        app.setView(app.getDefaultView());
        app.draw(gui);
        app.setView(view);
        app.draw(ww);
        app.display();
    }
    return EXIT_SUCCESS;
}
