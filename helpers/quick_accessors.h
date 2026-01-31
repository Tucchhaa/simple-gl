#pragma once

#include <memory>

class btDynamicsWorld;

namespace SimpleGL {

class Window;
class Input;

const std::unique_ptr<btDynamicsWorld>& dynamicsWorld();
const std::unique_ptr<Window>& window();
const std::unique_ptr<Input>& input();

}
