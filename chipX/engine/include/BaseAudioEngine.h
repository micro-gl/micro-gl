//
// Created by Tomer Shalev on 2019-06-12.
//

#pragma once


class BaseAudioEngine {
    BaseAudioEngine();
    ~BaseAudioEngine();

    virtual void init() = 0;

};
