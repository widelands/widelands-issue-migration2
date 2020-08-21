/*
 * Copyright (C) 2015-2020 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/single_player_game_controller.h"

#include <SDL_timer.h>

#include "ai/computer_player.h"
#include "base/multithreading.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "wlapplication_options.h"

SinglePlayerGameController::SinglePlayerGameController(Widelands::Game& game,
                                                       bool const useai,
                                                       Widelands::PlayerNumber const local)
   : game_(game),
     use_ai_(useai),
     lastframe_(SDL_GetTicks()),
     time_(game_.get_gametime()),
     speed_(get_config_natural("speed_of_new_game", 1000)),
     paused_(false),
     player_cmdserial_(0),
     local_(local) {
}

SinglePlayerGameController::~SinglePlayerGameController() {
	for (uint32_t i = 0; i < computerplayers_.size(); ++i) {
		if (computerplayers_[i].get() && computerplayers_[i]->running) {
			computerplayers_[i]->running = false;
			pthread_cancel(computerplayers_[i]->thread_id);
		}
	}
	computerplayers_.clear();
}

void SinglePlayerGameController::think() {
	uint32_t const curtime = SDL_GetTicks();
	int32_t frametime = curtime - lastframe_;
	lastframe_ = curtime;

	// prevent crazy frametimes
	if (frametime < 0) {
		frametime = 0;
	} else if (frametime > 1000) {
		frametime = 1000;
	}

	frametime = frametime * real_speed() / 1000;

	time_ = game_.get_gametime() + frametime;

	if (use_ai_ && game_.is_loaded()) {
		const Widelands::PlayerNumber nr_players = game_.map().get_nrplayers();
		iterate_players_existing(p, nr_players, game_, plr) if (p != local_) {
			// Deliberately use else-if's here to ensure the AI threads will be started with a
			// short delay. This fixes a race condition with the DefaultAI's lazy initialization.
			if (p > computerplayers_.size()) {
				computerplayers_.resize(p);
			} else if (!computerplayers_[p - 1].get()) {
				computerplayers_[p - 1].reset(new AIData());
			} else if (!computerplayers_[p - 1]->ai.get()) {
				computerplayers_[p - 1]->ai.reset(
				   ComputerPlayer::get_implementation(plr->get_ai())->instantiate(game_, p));
			} else if (!computerplayers_[p - 1]->running) {
				computerplayers_[p - 1]->running = true;
				if (int i = pthread_create(&computerplayers_[p - 1]->thread_id, NULL, &SinglePlayerGameController::runthread, computerplayers_[p - 1].get())) {
					throw wexception("PThread creation for AI %u failed with error code %d", static_cast<unsigned>(p), i);
				}
			}
		}
	}
}

void* SinglePlayerGameController::runthread(void* aidata) {
	assert(aidata);
	AIData& ai = *static_cast<AIData*>(aidata);
	uint32_t next_time = SDL_GetTicks() + kAIThinkDelay;
	for (;;) {
		const uint32_t time = SDL_GetTicks();
		if (time >= next_time) {
			MutexLock m;
			ai.ai->think();
		}
		const int32_t delay = next_time - SDL_GetTicks();
		if (delay > 0) {
			SDL_Delay(delay);
		}
	}
	// Thread will be killed when the game ends
	NEVER_HERE();
}

void SinglePlayerGameController::send_player_command(Widelands::PlayerCommand* pc) {
	pc->set_cmdserial(++player_cmdserial_);
	game_.enqueue_command(pc);
}

int32_t SinglePlayerGameController::get_frametime() {
	return time_ - game_.get_gametime();
}

GameController::GameType SinglePlayerGameController::get_game_type() {
	return GameController::GameType::kSingleplayer;
}

uint32_t SinglePlayerGameController::real_speed() {
	return paused_ ? 0 : speed_;
}

uint32_t SinglePlayerGameController::desired_speed() {
	return speed_;
}

void SinglePlayerGameController::set_desired_speed(uint32_t const speed) {
	speed_ = speed;
}

bool SinglePlayerGameController::is_paused() {
	return paused_;
}

void SinglePlayerGameController::set_paused(bool paused) {
	paused_ = paused;
}

void SinglePlayerGameController::report_result(uint8_t p_nr,
                                               Widelands::PlayerEndResult result,
                                               const std::string& info) {
	Widelands::PlayerEndStatus pes;
	Widelands::Player* player = game_.get_player(p_nr);
	assert(player);
	pes.player = player->player_number();
	pes.time = game_.get_gametime();
	pes.result = result;
	pes.info = info;
	game_.player_manager()->add_player_end_status(pes);
}
