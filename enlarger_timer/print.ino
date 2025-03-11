// Print exposure is a 2D array of values containing the base exposure, and
// dodge and burn exposures. The first index is the Channel (0 or 1).
// Second index < BASE_INDEX is dodge, Second index = 0 is the base exposure, 
// and second index > BASE_INDEX is burn.
#define BASE_INDEX 9 // Index of base exposure.
#define N_CHANNELS 2 // Number of print channels
#define N_SETTINGS 19 // Number of print settings (dodge, base exposure, burn)
uint8_t print_settings_[N_CHANNELS][N_SETTINGS] = {0};
uint8_t selected_channel_ = 0; // can be 0...(N_CHANNELS - 1)
uint8_t current_setting_ = BASE_INDEX; // can be 0..(N_SETTINGS - 1)
uint8_t current_print_settings_ = BASE_INDEX;

void PR_Init() {
	// Initial print exposure: 8 s base exposure
	print_settings_[0][BASE_INDEX] = 36;
	print_settings_[1][BASE_INDEX] = 36;
}

void PR_increment_setting_value(){
	if (current_setting_ == BASE_INDEX) {
		// Base exposure
		if (base_exposure_idx()	< kNExposures - selected_resolution_) {
			print_settings_[selected_channel_][BASE_INDEX] += selected_resolution_;
		} else {
			print_settings_[selected_channel_][BASE_INDEX] = kNExposures - 1;
		}
	} else if (current_setting_ < BASE_INDEX) {
		// Dodge exposure
		const uint8_t exposure_index = base_exposure_idx() - current_idx();
		if (exposure_index < selected_resolution_) {
			return;
		}
		uint16_t dodge_sum = 0;
		for (int i = 0; i < BASE_INDEX; ++i) {
			if (i != current_setting_ && print_settings_[selected_channel_][i] > 0) {
				const uint8_t exp_idx
					= base_exposure_idx() - print_settings_[selected_channel_][i];
				dodge_sum += base_exposure_ms() - kExposures[exp_idx];
			}
		}
		dodge_sum += base_exposure_ms()
			- kExposures[exposure_index - selected_resolution_];
		if (dodge_sum < base_exposure_ms()) {
			print_settings_[selected_channel_][current_setting_]
				+= selected_resolution_;
		}
	} else if (current_setting_ > BASE_INDEX) {
		// Burn exposure
		if (current_idx() + base_exposure_idx()
				< kNExposures - selected_resolution_) {
			print_settings_[selected_channel_][current_setting_]
				+= selected_resolution_;
		}
	}
}


void PR_decrement_setting_value(){
	if (current_setting_ == BASE_INDEX) {
		// Base exposure
		if (base_exposure_idx()	> selected_resolution_) {
			print_settings_[selected_channel_][BASE_INDEX] -= selected_resolution_;
		} else {
			print_settings_[selected_channel_][BASE_INDEX] = 0;
		}
	}	else {
		// Dodge or burn exposure
		if (current_idx()	> selected_resolution_) {
			print_settings_[selected_channel_][current_setting_]
				-= selected_resolution_;
		} else {
			print_settings_[selected_channel_][current_setting_] = 0;
		}
	}
}

void PR_increment_setting_index(){
	if ( current_setting_ <= BASE_INDEX ) {
		// Can always increment from a dodge setting or the base exposure
		++current_setting_;
	} else if ( current_setting_ < N_SETTINGS && current_idx() > 0 ) {
		// Can increment from a burn setting if the value is > 0
		++current_setting_;
	}
}

void PR_decrement_setting_index(){
	if ( current_setting_ >= BASE_INDEX ) {
		// Can always decrement from a burn setting or the base exposure
		--current_setting_;
	} else if ( current_setting_ > 0 && current_idx() > 0 ) {
		// Can decrement from a dodge setting if the value is > 0
		--current_setting_;
	}
}

void PR_display_setting_name(uint8_t setting){
	if ( setting == BASE_INDEX ) {
		display_text(' ', 'b', 'E');
	} else if ( setting < BASE_INDEX ) {
		display_text('d', 0, BASE_INDEX - setting);
	} else {
		display_text('b', 0, setting - BASE_INDEX);
	}
}

void PR_display_current_setting_name(){
	PR_display_setting_name(current_setting_);
}

void PR_display_current_setting_value(){
	if (current_setting_ == BASE_INDEX) {
		display_ms(base_exposure_ms());
	} else {
		display_int(current_idx());
	}
}

bool PR_is_last_exposure(uint8_t setting){
	// Last exposure: terminated by the number of available settings or by the
	// number of configured settings
	return (current_print_exposure_ == N_SETTINGS - 1) || (exposure_idx(current_print_exposure_ + 1) == 0);
}

void PR_set_channel(uint8_t channel){
	// Channel can be 0 or 1
	selected_channel_ = channel;
	// Reset to display the base exposure when changing channel
	current_setting_ = BASE_INDEX;
}

uint8_t PR_get_channel(){
	return selected_channel_;
}

uint8_t PR_get_first_print_exposure_index(){
	for (int i = 0; i < N_SETTINGS; ++i) {
		if (exposure_idx(i) > 0) {
			return i;
		}
	}
}

uint8_t base_exposure_idx() {
	// Return the setting for the base exposure (index of kExposures array)
	return print_settings_[selected_channel_][BASE_INDEX];
}

uint16_t base_exposure_ms() {
	// Return the setting for the base exposure value in milliseconds
	return kExposures[base_exposure_idx()];
}

uint8_t current_idx() {
	// Return the setting for the currently edited print exposure value
	return print_settings_[selected_channel_][current_setting_];
}

uint8_t exposure_idx(const uint8_t setting) {
	return print_settings_[selected_channel_][setting];
}

uint16_t PR_exposure_ms(const uint8_t setting) {
	if (setting < BASE_INDEX) {
		// Dodge
		return base_exposure_ms()
			- kExposures[base_exposure_idx() - exposure_idx(setting)];
	} else if (setting > BASE_INDEX) {
		// Burn
		return kExposures[base_exposure_idx() + exposure_idx(setting)]
			- base_exposure_ms();
	} else {
		uint16_t residual_base_exposure = base_exposure_ms();
		for (int i = 0; i < BASE_INDEX; ++i) {
			if (print_settings_[selected_channel_][i] > 0) {
				const uint8_t exp_idx	= base_exposure_idx() - exposure_idx(i);
				residual_base_exposure -= base_exposure_ms() - kExposures[exp_idx];
			}
		}
		return residual_base_exposure;
	}
}


