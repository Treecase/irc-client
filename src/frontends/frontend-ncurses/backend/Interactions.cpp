/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Interactions.hpp"


/* ===[ RespondInteraction ]=== */
RespondInteraction::RespondInteraction(Message const &msg)
:   _msg{msg}
{
}


void RespondInteraction::execute(Backend &backend) const
{
    backend.send_response(_msg);
}


/* ===[ PrintInteraction ]=== */
PrintInteraction::PrintInteraction(Message const &msg)
:   _msg{msg}
{
}


void PrintInteraction::execute(Backend &backend) const
{
    backend.get_active_channel().push_message(_msg);
}
