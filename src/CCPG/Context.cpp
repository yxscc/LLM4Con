#include "CCPG/CCPGNode.h"

std::string Context::toString() const {
    std::stringstream ss;
    ss << "Call Chain: \n";
    
    const auto& callStack = getCallStack();

    // 检查是否有调用栈帧，以及初始化迭代
    if (callStack.empty()) {
        return "No call frames available.\n";
    }

    auto it = callStack.begin();
    // 特殊处理第一个帧，假设它是 fork
    auto firstFrame = *it;
    if (firstFrame->getType() != ThreadAPIUtil::TYPE::FORK) {
        ss << "[FORK] " << firstFrame->getCPGNode()->getName() << " \n";
        ++it; // 移动到下一个元素
    }
    else {
        ss << "[FORK] " << firstFrame->getCPGNode()->getCode() 
        << "at ln" << firstFrame->getNodeLoc().getLineNumber() 
        << " in file " <<  firstFrame->getNodeLoc().getFileName() << " \n";
        ++it; // 移动到下一个元素
    }

    // 处理其余帧
    for (; it != callStack.end(); ++it) {
        auto frame = *it;
        if (frame) {
            ss << frame->getCPGNode()->getName() << " → ";
        }
    }

    // 移除最后一个多余的箭头
    std::string result = ss.str();
    size_t lastArrow = result.rfind("→ ");
    if (lastArrow != std::string::npos) {
        result.erase(lastArrow, 2); // 注意这里删除的字符数应当匹配 "→ "（包括空格）
    }

    return result;
}