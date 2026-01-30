-- Menu System Lua Script
-- Manages game menus (main menu, pause menu, settings)

local Menu = {}

-- Menu States
local MenuState = {
    MAIN_MENU = "main_menu",
    PAUSE_MENU = "pause_menu",
    SETTINGS = "settings",
    CREDITS = "credits",
    NONE = "none"
}

function Menu:OnCreate()
    self.currentState = MenuState.NONE
    self.selectedOption = 0
    self.options = {}
    
    print("[Lua] Menu system created")
end

function Menu:ShowMainMenu()
    self.currentState = MenuState.MAIN_MENU
    self.options = {
        "New Game",
        "Continue",
        "Settings",
        "Credits",
        "Exit"
    }
    self.selectedOption = 0
    
    print("[Lua] Showing main menu")
end

function Menu:ShowPauseMenu()
    self.currentState = MenuState.PAUSE_MENU
    self.options = {
        "Resume",
        "Settings",
        "Main Menu",
        "Exit"
    }
    self.selectedOption = 0
    
    print("[Lua] Showing pause menu")
end

function Menu:ShowSettings()
    self.currentState = MenuState.SETTINGS
    self.options = {
        "Graphics",
        "Audio",
        "Controls",
        "Back"
    }
    self.selectedOption = 0
    
    print("[Lua] Showing settings menu")
end

function Menu:Hide()
    self.currentState = MenuState.NONE
    self.options = {}
    print("[Lua] Menu hidden")
end

function Menu:OnUpdate(dt)
    if self.currentState == MenuState.NONE then
        return
    end
    
    -- Handle menu navigation
    -- In production, respond to input events
end

function Menu:NavigateUp()
    if #self.options > 0 then
        self.selectedOption = (self.selectedOption - 1) % #self.options
        print("[Lua] Menu navigation: " .. self.options[self.selectedOption + 1])
    end
end

function Menu:NavigateDown()
    if #self.options > 0 then
        self.selectedOption = (self.selectedOption + 1) % #self.options
        print("[Lua] Menu navigation: " .. self.options[self.selectedOption + 1])
    end
end

function Menu:SelectOption()
    if #self.options > 0 then
        local option = self.options[self.selectedOption + 1]
        print("[Lua] Menu option selected: " .. option)
        
        -- Handle selection
        self:HandleSelection(option)
    end
end

function Menu:HandleSelection(option)
    -- Main menu
    if option == "New Game" then
        print("[Lua] Starting new game...")
        self:Hide()
    elseif option == "Continue" then
        print("[Lua] Continuing game...")
        self:Hide()
    elseif option == "Settings" then
        self:ShowSettings()
    elseif option == "Resume" then
        print("[Lua] Resuming game...")
        self:Hide()
    elseif option == "Exit" then
        print("[Lua] Exiting game...")
        -- Trigger game exit
    end
end

return Menu
