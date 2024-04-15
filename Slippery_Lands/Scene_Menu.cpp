#include "Scene_Menu.h"
#include "Scene_Ice.h"
#include <memory>

void Scene_Menu::onEnd()
{
	m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}



void Scene_Menu:: init()
{
    registerAction(sf::Keyboard::W,			"UP");
    registerAction(sf::Keyboard::Up,		"UP");
    registerAction(sf::Keyboard::S,			"DOWN");
    registerAction(sf::Keyboard::Down,	 	"DOWN");
	registerAction(sf::Keyboard::D,			"PLAY");
	registerAction(sf::Keyboard::Escape,	"QUIT");

	m_title = "Slippery    Lands";
	m_menuStrings.push_back("Level 1");
	m_menuStrings.push_back("Level 2");
	m_menuStrings.push_back("Level 3");
	m_menuStrings.push_back("Level 4");
	m_menuStrings.push_back("Credits");


	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level2.txt");
	m_levelPaths.push_back("../assets/level3.txt");
	m_levelPaths.push_back("../assets/level4.txt");


	m_menuText.setFont(Assets::getInstance().getFont("Arcade"));

	const size_t CHAR_SIZE{ 64 };
	m_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	m_entityManager.update();
}


void Scene_Menu::sRender()
{
	 
	sf::View view = m_game->window().getView();
	view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
	m_game->window().setView(view);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 0, 0);
	    // draw bkg first
    /*for (auto e : m_entityManager.getEntities("bkg")) {
        if (e->getComponent<CSprite>().has) {
            auto&sprite = e->getComponent<CSprite>().sprite;
            m_game->window().draw(sprite);
        }
    }*/
	static const sf::Color backgroundColor(155, 155, 155);

	sf::Text footer("UP: W    DOWN: S   PLAY:D    QUIT: ESC", 
		Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 700);

	m_game->window().clear(backgroundColor);

	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setPosition(10, 10);
	m_game->window().draw(m_menuText);

	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setPosition(32, 32 + (i+1) * 96);
		m_menuText.setString(m_menuStrings.at(i));
		m_game->window().draw(m_menuText);
	} 

	m_game->window().draw(footer);
	//m_game->window().display();

}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		} 
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			m_game->changeScene("PLAY", std::make_shared<Scene_Ice>(m_game, m_levelPaths[m_menuIndex]));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
