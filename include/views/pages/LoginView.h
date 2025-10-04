/**
 * @file LoginView.h
 * @brief Login page view with user selection grid - MVC View Layer
 * 
 * User selection screen displaying user profiles in hexagonal grid.
 * Part of MVC architecture - View layer (page).
 */

#ifndef LOGIN_VIEW_H
#define LOGIN_VIEW_H

#include "controllers/NavigationController.h"
#include "views/layouts/HexagonalGrid.h"
#include "services/DatabaseService.h"
#include "services/AuthService.h"

/**
 * @class LoginView
 * @brief User selection/login page
 * 
 * Features:
 * - Hexagonal grid of user profiles
 * - Profile image + username
 * - Tap to login
 * - Drag to scroll (if many users)
 * - Visual feedback on selection
 */
class LoginView : public PageView {
public:
    /**
     * @brief Constructor
     */
    LoginView();

    /**
     * @brief Destructor
     */
    ~LoginView();

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Login"; }

private:
    void loadUsers();
    void createUserIcon(User* user);
    void onUserSelected(int userId);
    
    HexagonalGrid* m_grid;
    TouchPoint m_lastTouch;
    bool m_isDragging;
    int m_selectedUserId;
};

/**
 * @brief Factory function for LoginView
 * @return Pointer to new LoginView instance
 */
PageView* createLoginView();

#endif // LOGIN_VIEW_H


