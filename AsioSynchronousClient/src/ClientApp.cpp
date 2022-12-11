#include "../inc/ClientApp.h"
#include "../inc/Util.h"

#include <iostream>
#include <chrono>


using namespace std::chrono_literals;

ClientApp::ClientApp() : 
    m_client("127.0.0.1", 1325, *this)  //18.102.79.209
{
    m_app = App::Create();
    m_app->set_listener(this);

    m_window = Window::Create(m_app->main_monitor(), 1080, 720,
        false, kWindowFlags_Titled | kWindowFlags_Maximizable);
    m_window->SetTitle("Prepelica");
    m_window->set_listener(this);

    m_app->set_window(*m_window.get());
    m_userView = Overlay::Create(*m_window.get(), 1, 1, 0, 0);
    m_userView->view()->LoadURL("file:///loginForm.html");
    m_userView->view()->Focus();
    OnResize(m_window->width(), m_window->height());
    m_userView->view()->set_view_listener(this);
    m_userView->view()->set_load_listener(this);
    m_client.start();
}


void ClientApp::OnResize(uint32_t width, uint32_t height) {
    m_userView->Resize((uint32_t)width, height);
    m_userView->MoveTo(0, 0);
}

std::string ClientApp::jsStringToStdString(JSString&& jsString)
{
    return std::string(static_cast<ultralight::String>(jsString).utf8().data());
}

void ClientApp::sendRegistrationMessage(const JSObject& jsObject, const JSArgs& args)
{
    m_client.sendRegistrationMessage(jsStringToStdString(args[0]), jsStringToStdString(args[1]), jsStringToStdString(args[2]), jsStringToStdString(args[3]));
}

void ClientApp::sendLoginMessage(const JSObject& jsObject, const JSArgs& args)
{
    m_client.sendLoginMessage(jsStringToStdString(args[0]), jsStringToStdString(args[1]));
}

void ClientApp::sendTextMessage(const JSObject& jsObject, const JSArgs& args)
{
    m_client.sendTextMessage(jsStringToStdString(args[0]), jsStringToStdString(args[1]));
}

void ClientApp::goToMainScreen()
{
    m_userView->view()->LoadURL("file:///index.html");
}

void ClientApp::goToLoginScreenAfterSuccessfulRegistration()
{
    m_userView->view()->LoadURL("file:///loginFormAfterSuccessfulRegistration.html");
}

void ClientApp::OnUpdate()
{
    if (m_clientState == 1) // client registered
    {
        goToLoginScreenAfterSuccessfulRegistration();
        m_clientState = 0;
    }
    else if (m_clientState == 2) // client logged in 
    {
        goToMainScreen();
        m_clientState = 0;
    }
    if (!m_messagesToDisplay.empty())
    {
        handleMessage(m_messagesToDisplay.popFront());
    }
}

void ClientApp::OnChangeCursor(ultralight::View* caller, Cursor cursor)
{
    m_window->SetCursor(cursor);
}

void ClientApp::handleMessage(const Message& message)
{
    if (message.messageType == MessageType::TextMessage)
    {
        std::string username, textMessage;
        Util::extractUsernameAndMessage(message.messageContent, username, textMessage);
        displayMessage(username, textMessage);
    }
    else if (message.messageType == MessageType::FriendRequest)
    {
        if (message.messageContent[0] == 'T')
            m_friendExists({});
        else
            m_friendDoesNotExist({});
    }
}

void ClientApp::SetState(int state)
{
    m_clientState = state;
}

void ClientApp::displayMessage(const std::string& fromUser, const std::string& textMessage)
{
    m_displayMessage({ JSString(fromUser.c_str()), JSString(textMessage.c_str()) });
}

void ClientApp::pushMessage(const Message& message)
{
    m_messagesToDisplay.insertBack(message);
}

void ClientApp::checkIfFriendExists(const JSObject& jsObject, const JSArgs& args)
{
    m_client.checkIfFriendExists(jsStringToStdString(args[0]));
}

void ClientApp::OnDOMReady(ultralight::View* caller,
    uint64_t frame_ide,
    bool is_main_frame,
    const ultralight::String& url) 
{
    Ref<JSContext> context = caller->LockJSContext();
    SetJSContext(context.get());

    JSObject global = JSGlobalObject();

    global["sendRegistrationMessage"] = BindJSCallback(&ClientApp::sendRegistrationMessage);
    global["sendLoginMessage"] = BindJSCallback(&ClientApp::sendLoginMessage);
    global["sendTextMessage"] = BindJSCallback(&ClientApp::sendTextMessage);
    global["checkIfFriendExists"] = BindJSCallback(&ClientApp::checkIfFriendExists);
    m_displayMessage = global["displayMessage"];
    m_friendExists = global["friendExists"];
    m_friendDoesNotExist = global["friendDoesNotExist"];
}


void ClientApp::OnClose() {}

void ClientApp::run() 
{
    m_app->Run();
}
