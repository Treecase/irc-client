/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_INTERACTIONS_HPP
#define FRONTENDNCURSES_INTERACTIONS_HPP

#include "Backend.hpp"


class Interaction
{
public:
    virtual ~Interaction()=default;

    virtual void execute(Backend &backend) const=0;
};


class RespondInteraction : public Interaction
{
    Message const _msg;
public:
    RespondInteraction(Message const &msg);
    virtual ~RespondInteraction()=default;

    virtual void execute(Backend &backend) const override;
};


class PrintInteraction : public Interaction
{
    Message const _msg;
public:
    PrintInteraction(Message const &msg);
    virtual ~PrintInteraction()=default;

    virtual void execute(Backend &backend) const override;
};


#endif
