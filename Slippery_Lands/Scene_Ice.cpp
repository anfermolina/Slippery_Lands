

#include <fstream>
#include <iostream>

#include "Scene_Ice.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>

namespace {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1, 4); // distribution in range [1, 4]
}


Scene_Ice::Scene_Ice(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_worldView(gameEngine->window().getDefaultView())
    , m_remainingTime(sf::seconds(120.f))
{
    loadLevel(levelPath);
    registerActions();

    auto pos = m_worldView.getSize();

    pos.x = pos.x / 2.f;
    pos.y -= 72.f;
    spawnPlayer(pos);

    MusicPlayer::getInstance().play("gameTheme");
    MusicPlayer::getInstance().setVolume(50);
}


void Scene_Ice::init(const std::string& path) {
}

void Scene_Ice::sMovement(sf::Time dt) {
    playerMovement();

    // move all objects
    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CInput>())
            continue; // player is moved in playerMovement
        if (e->hasComponent<CTransform>()) {
            auto& tfm = e->getComponent<CTransform>();

            tfm.pos += tfm.vel * dt.asSeconds();
            tfm.angle += tfm.angVel * dt.asSeconds();
        }
    }
}


void Scene_Ice::registerActions() {
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "BACK");
    registerAction(sf::Keyboard::Q, "QUIT");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Right, "RIGHT");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");

    registerAction(sf::Keyboard::G, "TOGGLE_GRID");
}


void Scene_Ice::onEnd() {
    m_game->changeScene("MENU", nullptr, false);
}

void Scene_Ice::playerMovement() {
    // no movement if player is dead
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    bool overSnow = false;
    for (auto e : m_entityManager.getEntities()) {
        sf::Vector2f overlap = Physics::getOverlap(m_player, e);
        bool thereIsCollision = overlap.x > 0.f && overlap.y > 0.f;

        if (thereIsCollision && e->getTag() == "Snow") {
            overSnow = true;
            break;
        }
    }

    auto& dir = m_player->getComponent<CInput>().dir;
    auto& pos = m_player->getComponent<CTransform>().pos;

    std::string stepSndRandom = std::to_string(dist6(rng));

    if (dir & CInput::UP) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
        if (overSnow) {
            pos.y -= 10.f;
            SoundPlayer::getInstance().play("Step" + stepSndRandom);
        }
        else pos.y -= 2.f;
    }
    if (dir & CInput::DOWN) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
        if (overSnow) {
            pos.y += 10.f;
            SoundPlayer::getInstance().play("Step" + stepSndRandom);
        }
        else pos.y += 2.f;
    }

    if (dir & CInput::LEFT) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
        if (overSnow) {
            pos.x -= 10.f;
            SoundPlayer::getInstance().play("Step" + stepSndRandom);
        }
        else pos.x -= 2.f;
    }

    if (dir & CInput::RIGHT) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
        if (overSnow) {
            pos.x += 10.f;
            SoundPlayer::getInstance().play("Step" + stepSndRandom);
        }
        else pos.x += 2.f;
    }
    if (overSnow) dir = 0;
}


void Scene_Ice::sRender() {
    m_game->window().setView(m_worldView);

    // draw bkg first
    /*for (auto e : m_entityManager.getEntities("bkg")) {
        if (e->getComponent<CSprite>().has) {
            auto&sprite = e->getComponent<CSprite>().sprite;
            m_game->window().draw(sprite);
        }
    }*/
    static const sf::Color backgroundColor(100, 100, 255);
    m_game->window().clear(backgroundColor);

    // First Background (Ice, walls and door)
    for (auto& e : m_entityManager.getEntities()) {
        if (e->getTag() == "Ice1" || e->getTag() == "Ice2" || e->getTag() == "Ice3" || e->getTag() == "Ice4" || e->getTag() == "Ice5"
            || e->getTag() == "Wall" || e->getTag() == "Door" || e->getTag() == "DoorOp") {
            if (e->hasComponent<CSprite>()) {
                // Draw Sprites
                auto& sprt = e->getComponent<CSprite>().sprite;
                auto& tfm = e->getComponent<CTransform>();
                sprt.setPosition(tfm.pos.x, tfm.pos.y);
                sprt.setRotation(tfm.angle);
                m_game->window().draw(sprt);
            }
        }
    }

    // Then the rest (Player, trees, box, etc)
    for (auto& e : m_entityManager.getEntities()) {
        if (e->getTag() != "Ice1" && e->getTag() != "Ice2" && e->getTag() != "Ice3" && e->getTag() != "Ice4" && e->getTag() != "Ice5"
            && e->getTag() != "Wall" && e->getTag() != "Door" && e->getTag() != "DoorOp") {
            if (e->hasComponent<CSprite>()) {
                // Draw Sprites
                auto& sprt = e->getComponent<CSprite>().sprite;
                auto& tfm = e->getComponent<CTransform>();
                sprt.setPosition(tfm.pos.x, tfm.pos.y);
                sprt.setRotation(tfm.angle);
                m_game->window().draw(sprt);
            }

            if (e->hasComponent<CAnimation>()) {

                // Draw Animation
                auto& anim = e->getComponent<CAnimation>().animation;
                auto& tfm = e->getComponent<CTransform>();
                anim.getSprite().setPosition(tfm.pos);
                anim.getSprite().setRotation(tfm.angle);
                m_game->window().draw(anim.getSprite());

                if (m_drawAABB) {
                    if (e->hasComponent<CBoundingBox>()) {
                        auto box = e->getComponent<CBoundingBox>();
                        sf::RectangleShape rect;
                        rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
                        centerOrigin(rect);
                        rect.setPosition(e->getComponent<CTransform>().pos);
                        rect.setFillColor(sf::Color(0, 0, 0, 0));
                        rect.setOutlineColor(sf::Color{ 0, 255, 0 });
                        rect.setOutlineThickness(2.f);
                        m_game->window().draw(rect);
                    }
                }
            }
        }
    }

    sf::Text gridText;
    gridText.setFont(Assets::getInstance().getFont("Arial"));
    gridText.setCharacterSize(10);
    int margin = 36;
    if (m_drawGrid) {
        for (auto e : m_entityManager.getEntities()) {
            if (e->getComponent<CSprite>().has) {
                auto pos = e->getComponent<CTransform>().pos;
                pos.x -= margin;
                pos.y -= margin;

                sf::Vertex line[] =
                {
                    sf::Vertex(sf::Vector2f(pos.x, pos.y)),
                    sf::Vertex(sf::Vector2f(pos.x + 72.f, pos.y))
                };
                m_game->window().draw(line, 2, sf::Lines);

                sf::Vertex line2[] =
                {
                    sf::Vertex(sf::Vector2f(pos.x, pos.y)),
                    sf::Vertex(sf::Vector2f(pos.x, pos.y + 72.f))
                };
                m_game->window().draw(line2, 2, sf::Lines);

                std::string text = "(" + std::to_string((int)(pos.x / 72.f)) + "," + std::to_string((int)(pos.y / 72.f)) + ")";

                gridText.setString(text);
                gridText.setPosition(pos);
                m_game->window().draw(gridText);
            }
        }
    }

    // Draw timer text
    sf::Text timerText;
    timerText.setFont(Assets::getInstance().getFont("Arial"));
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(10.f, 10.f); // Position the text in the top-left corner

    // Format remaining time as MM:SS
    int remainingSeconds = static_cast<int>(m_remainingTime.asSeconds());
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;
    std::string timerString = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);

    timerText.setString("Time: " + timerString);
    m_game->window().draw(timerText);
}


void Scene_Ice::update(sf::Time dt) {
    sUpdate(dt);
}

void Scene_Ice::sDoAction(const Command& action) {
    // On Key Press
    if (action.type() == "START") {
        if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
        else if (action.name() == "QUIT") { m_game->quitLevel(); }
        else if (action.name() == "BACK") { m_game->backLevel(); }

        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

        bool overSnow = false;
        for (auto e : m_entityManager.getEntities()) {
            sf::Vector2f overlap = Physics::getOverlap(m_player, e);
            bool thereIsCollision = overlap.x > 0.f && overlap.y > 0.f;

            if (thereIsCollision && e->getTag() == "Snow") {
                overSnow = true;
                break;
            }
        }

        auto& input = m_player->getComponent<CInput>();

        // Player control
        if (m_player->getComponent<CInput>().dir == 0) { // only move if it's still
            if (action.name() == "LEFT") { m_player->getComponent<CInput>().dir = CInput::LEFT; }
            else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
            else if (action.name() == "UP") { m_player->getComponent<CInput>().dir = CInput::UP; }
            else if (action.name() == "DOWN") { m_player->getComponent<CInput>().dir = CInput::DOWN; }
        }
    }
}


void Scene_Ice::spawnPlayer(sf::Vector2f pos) {
    m_player = m_entityManager.addEntity("player");

    int margin = 36;
    m_player->addComponent<CTransform>(sf::Vector2f(margin + pos.x, margin + pos.y));
    m_player->addComponent<CBoundingBox>(sf::Vector2f(16.f, 16.f));
    m_player->addComponent<CInput>();
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
}


sf::FloatRect Scene_Ice::getViewBounds() {
    return sf::FloatRect();
}

void Scene_Ice::sCollisions() {
    auto& transf = m_player->getComponent<CTransform>();
    for (auto e : m_entityManager.getEntities())
    {
        auto& tile_transf = e->getComponent<CTransform>();
        sf::Vector2f overlap = Physics::getOverlap(m_player, e);
        bool thereIsCollision = overlap.x > 0.f && overlap.y > 0.f;

        // Player with Tile
        if (thereIsCollision) {
            if (e->getTag() == "Rock" || e->getTag() == "Tree" || e->getTag() == "Wall" || e->getTag() == "Limit" || e->getTag() == "Door") {
                m_player->getComponent<CInput>().dir = 0;
                if (overlap.x < overlap.y) { // only compensate on the shorter axis
                    if (transf.pos.x < tile_transf.pos.x)
                        transf.pos.x -= overlap.x;
                    else transf.pos.x += overlap.x;
                }
                else {
                    if (transf.pos.y < tile_transf.pos.y)  // bottom
                        transf.pos.y -= overlap.y;
                    else // top
                        transf.pos.y += overlap.y;
                }
            }
            else if (e->getTag() == "Box") { // add overlap to box
                if (overlap.x < overlap.y) {
                    if (transf.pos.x < tile_transf.pos.x)
                        tile_transf.pos.x += overlap.x;
                    else tile_transf.pos.x -= overlap.x;
                }
                else {
                    if (transf.pos.y < tile_transf.pos.y)
                        tile_transf.pos.y += overlap.y;
                    else tile_transf.pos.y -= overlap.y;
                }
            }
        }
        // Box collision
        if (e->getTag() == "Box") {
            for (auto e2 : m_entityManager.getEntities()) {
                auto& e2_transf = e2->getComponent<CTransform>();
                overlap = Physics::getOverlap(e, e2);
                thereIsCollision = overlap.x > 0.f && overlap.y > 0.f;

                if (thereIsCollision) {
                    if (e2->getTag() == "Rock" || e2->getTag() == "Tree" || e2->getTag() == "Snow" || e2->getTag() == "Wall" || e2->getTag() == "Limit") {
                        m_player->getComponent<CInput>().dir = 0;
                        if (overlap.x < overlap.y) { // only compensate on the shorter axis
                            if (tile_transf.pos.x < e2_transf.pos.x)
                                tile_transf.pos.x -= overlap.x;
                            else tile_transf.pos.x += overlap.x;
                        }
                        else {
                            if (tile_transf.pos.y < e2_transf.pos.y)  // bottom
                                tile_transf.pos.y -= overlap.y;
                            else // top
                                tile_transf.pos.y += overlap.y;
                        }
                    }
                    else if (e2->getTag() == "Swch") {
                        e2->addComponent<CSprite>(Assets::getInstance().getTexture("SwPrsd")).sprite;
                        for (auto e3 : m_entityManager.getEntities()) {
                            if (e3->getTag() == "Door") {
                                e3->addComponent<CSprite>(Assets::getInstance().getTexture("DoorOp")).sprite;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }


}


void Scene_Ice::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    m_entityManager.update();

    m_remainingTime -= dt;

    if (m_remainingTime <= sf::Time::Zero) {
        m_game->quitLevel();
    }

    if (m_isPaused)
        return;

    sAnimation(dt);
    sMovement(dt);
    sCollisions();
}


void Scene_Ice::sAnimation(sf::Time dt) {
    auto list = m_entityManager.getEntities();
    for (auto e : m_entityManager.getEntities()) {
        // update all animations
        if (e->hasComponent<CAnimation>()) {
            auto& anim = e->getComponent<CAnimation>();
            anim.animation.update(dt);
            // do nothing if animation has ended
        }
    }
}


void Scene_Ice::checkPlayerState() {
}

void Scene_Ice::loadLevel(const std::string& path) {
    std::ifstream config(path);
    if (config.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        config.close();
        exit(1);
    }

    std::string token{ "" };
    config >> token;
    while (!config.eof()) {
        if (token == "Bkg") {
            std::string name;
            sf::Vector2f pos;
            config >> name >> pos.x >> pos.y;
            auto e = m_entityManager.addEntity("bkg");

            // for background, no textureRect its just the whole texture
            // and no center origin, position by top left corner
            // stationary so no CTransfrom required.
            auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
            sprite.setOrigin(0.f, 0.f);
            sprite.setPosition(pos);
        }
        else if (token[0] == '#') {
            std::cout << token;
        }
        else if (token == "Tile") {
            int x, y;
            std::string name;
            config >> name >> x >> y;

            int margin = 36;

            auto tile = m_entityManager.addEntity(name);
            tile->addComponent<CSprite>(Assets::getInstance().getTexture(name));
            tile->addComponent<CTransform>(sf::Vector2f(margin + 72.f * x, margin + 72.f * y));
            tile->addComponent<CBoundingBox>(sf::Vector2f(72.f, 72.f));
        }
        config >> token;
    }

    config.close();

    // ADD MAP LIMITS
    auto viewSize = m_worldView.getSize();
    auto l = m_entityManager.addEntity("Limit");
    l->addComponent<CTransform>(sf::Vector2f(0.f, viewSize.y / 2.f));
    l->addComponent<CBoundingBox>(sf::Vector2f(6.f, viewSize.y));
    auto r = m_entityManager.addEntity("Limit");
    r->addComponent<CTransform>(sf::Vector2f(viewSize.x, viewSize.y / 2.f));
    r->addComponent<CBoundingBox>(sf::Vector2f(6.f, viewSize.y));
    auto b = m_entityManager.addEntity("Limit");
    b->addComponent<CTransform>(sf::Vector2f(viewSize.x / 2.f, viewSize.y));
    b->addComponent<CBoundingBox>(sf::Vector2f(viewSize.x, 6.f));
}
