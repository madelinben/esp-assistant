/**
 * @file LoginView.cpp
 * @brief Implementation of LoginView
 */

#include "views/pages/LoginView.h"
#include "controllers/NavigationController.h"
#include "hardware/display/DisplayDriver.h"

// Global callback helpers for user selection
struct UserCallbackData {
    int userId;
    LoginView* view;
};

static std::vector<UserCallbackData*> g_userCallbacks;

LoginView::LoginView()
    : m_grid(nullptr)
    , m_isDragging(false)
    , m_selectedUserId(-1) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
}

LoginView::~LoginView() {
    if (m_grid) {
        delete m_grid;
    }
    
    // Clean up callbacks
    for (auto* data : g_userCallbacks) {
        delete data;
    }
    g_userCallbacks.clear();
}

void LoginView::onEnter() {
    DEBUG_PRINTLN("[LoginView] Entering...");
    
    m_isActive = true;
    m_selectedUserId = -1;

    // Create hexagonal grid
    if (!m_grid) {
        m_grid = new HexagonalGrid(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    }

    // Load users from database
    loadUsers();

    DEBUG_PRINTLN("[LoginView] Entered");
}

void LoginView::onExit() {
    DEBUG_PRINTLN("[LoginView] Exiting...");
    m_isActive = false;
}

void LoginView::update() {
    // Update logic if needed
}

void LoginView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_BLUE);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, TFT_BLUE);

    // Draw title
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->drawString("SELECT USER", SCREEN_CENTER_X, 20);

    // Draw hexagonal grid of users
    if (m_grid) {
        m_grid->render();
    }

    // Draw instruction text
    sprite->setTextDatum(BC_DATUM);
    sprite->setTextColor(TFT_DARKGREY);
    sprite->drawString("Tap to login", SCREEN_CENTER_X, SCREEN_HEIGHT - 20);
}

void LoginView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    TouchPoint currentTouch = touchCtrl.getCurrentTouch();

    switch (event) {
        case TouchEvent::TAP:
            if (m_grid) {
                m_grid->handleTap(currentTouch.x, currentTouch.y);
            }
            break;

        case TouchEvent::DRAG_START:
            m_isDragging = true;
            break;

        case TouchEvent::DRAG_MOVE:
            if (m_isDragging && m_grid) {
                int16_t deltaX = currentTouch.x - m_lastTouch.x;
                int16_t deltaY = currentTouch.y - m_lastTouch.y;
                m_grid->handleDrag(deltaX, deltaY);
            }
            break;

        case TouchEvent::DRAG_END:
            m_isDragging = false;
            break;

        default:
            break;
    }

    m_lastTouch = currentTouch;
}

void LoginView::loadUsers() {
    if (!m_grid) return;

    DEBUG_PRINTLN("[LoginView] Loading users...");

    // Clear existing users
    m_grid->clear();
    
    // Clean up old callbacks
    for (auto* data : g_userCallbacks) {
        delete data;
    }
    g_userCallbacks.clear();

    // Get all users from database
    int userCount = 0;
    User** users = DatabaseService::getInstance().getAllUsers(userCount);

    if (userCount == 0) {
        DEBUG_PRINTLN("[LoginView] No users found!");
        
        // TODO: Show "Create User" button
        GridItem item;
        item.label = "Create User";
        item.icon = nullptr;
        item.backgroundColor = TFT_DARKGREEN;
        item.onTap = []() {
            DEBUG_PRINTLN("[LoginView] Create user tapped");
            // TODO: Navigate to user creation page
        };
        item.userData = nullptr;
        m_grid->addItem(item);
    } else {
        // Load users into grid
        for (int i = 0; i < userCount; i++) {
            createUserIcon(users[i]);
            delete users[i];  // Clean up
        }
        delete[] users;
    }

    DEBUG_PRINTF("[LoginView] Loaded %d users\n", m_grid->getItemCount());
}

void LoginView::createUserIcon(User* user) {
    if (!user || !user->isValid()) return;

    // Create callback data
    UserCallbackData* callbackData = new UserCallbackData();
    callbackData->userId = user->getId();
    callbackData->view = this;
    g_userCallbacks.push_back(callbackData);

    // Create grid item
    GridItem item;
    item.label = user->getUsername().c_str();
    item.icon = nullptr;  // TODO: Load profile image
    item.backgroundColor = TFT_DARKBLUE;
    item.userData = callbackData;
    
    // Set callback
    item.onTap = [callbackData]() {
        DEBUG_PRINTF("[LoginView] User tapped: %d\n", callbackData->userId);
        
        // Attempt login
        if (AuthService::getInstance().login(callbackData->userId)) {
            DEBUG_PRINTLN("[LoginView] Login successful!");
            
            // Navigate to home
            NavigationController::getInstance().navigateTo("/", true);
        } else {
            DEBUG_PRINTLN("[LoginView] Login failed!");
            // TODO: Show error message
        }
    };

    m_grid->addItem(item);
}

void LoginView::onUserSelected(int userId) {
    m_selectedUserId = userId;
    
    DEBUG_PRINTF("[LoginView] User selected: %d\n", userId);
    
    // Attempt login
    if (AuthService::getInstance().login(userId)) {
        DEBUG_PRINTLN("[LoginView] Login successful!");
        
        // Navigate to home
        NavigationController::getInstance().navigateTo("/", true);
    } else {
        DEBUG_PRINTLN("[LoginView] Login failed!");
    }
}

PageView* createLoginView() {
    return new LoginView();
}


