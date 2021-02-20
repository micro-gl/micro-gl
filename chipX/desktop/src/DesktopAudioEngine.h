//
// Created by Tomer Shalev on 2019-06-13.
//

#pragma once

#include <BaseAudioEngine.h>
#include <portaudio.h>
#include <iostream>
#include <cstdlib>

class DesktopAudioEngine : public BaseAudioEngine {
public:
    DesktopAudioEngine();
    virtual ~DesktopAudioEngine();
    void write();
    void init() override;

private:
};

