#define PC_SEG_A B00000001
#define PC_SEG_B B00000010
#define PC_SEG_C B00100000
#define PC_SEG_D B00001000
#define PC_SEG_E B00010000
#define PC_SEG_F B00000100
#define PC_SEG_MASK B00111111
#define PD_SEG_G B00000001
#define PD_SEG_DP B00000010
#define PD_SEG_MASK B00000011
#define PD_BUTTON_IN_1 B00000100
#define PD_BUTTON_IN_2 B00001000
#define PD_BUTTON_IN_3 B00010000
#define PD_BUTTON_IN_4 B00100000
#define PD_BUTTON_IN_MASK B00111100
#define PD_RELAY_CTRL B10000000
#define PD_BUZZER_CTRL B01000000
#define PB_SEG_1 B00000010
#define PB_SEG_2 B00000100
#define PB_SEG_3 B00001000
#define PB_SEG_MASK B00001110
#define PB_BUTTON_V_1 B00000001
#define PB_BUTTON_V_2 B00010000
#define PB_BUTTON_V_3 B00100000
#define PB_BUTTON_V_MASK B00110001
#define TCCR1A_SEG_1 B10000000 // _BV(COM1A1) "Clear OC1A on compare match"
#define TCCR1A_SEG_2 B00100000 // _BV(COM1B1) "Clear OC1B on compare match"
#define TCCR1A_SEG_MASK B10100000
#define TCCR2A_SEG_3 B10000000 // _BV(COM2A1) "Clear OC2A on compare match"
#define TCCR2A_SEG_MASK B10000000

#define N_TEST_STRIPS 7
#define N_DEBOUNCE_ITERS 5
#define START_DELAY 2000
#define BUZZER_ENABLED 0
#define RELAY_ENABLED 1

enum State{
	kInit,
	kTestStripIdle,
	kTestStripOverrideOn,
	kTestStripIncExp,
	kTestStripDecExp,
	kTestStripIncDisp,
	kTestStripDecDisp,
	kTestStripStart,
	kTestStripDelay,
	kTestStripExp,
	kTestStripCancel,
	kPrintIdle,
	kPrintOverrideOn,
	kPrintIncExp,
	kPrintDecExp,
	kPrintUpPressed,
	kPrintDownPressed,
	kPrintDispSetting,
	kPrintIncSetting,
	kPrintDecSetting,
	kPrintStart,
	kPrintDelay,
	kPrintExposure,
	kPrintCancel,
	kError,
};
State state_ = State::kInit;

struct segment{
	char character = ' ';
	bool dot = false;
	const uint8_t segment;
};
segment segment_1_ = {' ', false, 1};
segment segment_2_ = {'O', false, 2};
segment segment_3_ = {'n', false, 3};
long event_start_ms_ = 0;

struct buttons_switches{
	bool plus_pressed = false;
	bool minus_pressed = false;
	bool up_pressed = false;
	bool down_pressed = false;
	bool start_pressed = false;
	bool stops_oneoneth_active = false;
	bool stops_onesixth_active = false;
	bool mode_active = false;
	bool channel_active = false;
	bool override_active = false;
};
buttons_switches buttons_;

struct buttons_switches_debounce{
	uint8_t plus_low_count = 0;
	uint8_t minus_low_count = 0;
	uint8_t up_low_count = 0;
	uint8_t down_low_count = 0;
	uint8_t start_low_count = 0;
	uint8_t stops_oneoneth_low_count = 0;
	uint8_t stops_onesixth_low_count = 0;
	uint8_t mode_low_count = 0;
	uint8_t channel_low_count = 0;
	uint8_t override_low_count = 0;
};
buttons_switches_debounce buttons_debounce_;

// LUT of exposures in 1/6 stops, milliseconds
const uint8_t kNExposures = 73;
const uint16_t kExposures[kNExposures] = {
	1000,
	1059,
	1122,
	1189,
	1259,
	1334,
	1414,
	1498,
	1587,
	1681,
	1781,
	1887,
	2000,
	2118,
	2244,
	2378,
	2519,
	2669,
	2828,
	2996,
	3174,
	3363,
	3563,
	3775,
	4000,
	4237,
	4489,
	4756,
	5039,
	5339,
	5656,
	5993,
	6349,
	6727,
	7127,
	7550,
	8000, //
	8475,
	8979,
	9513,
	10079, //
	10678,
	11313,
	11986, 
	12699, //
	13454,
	14254,
	15101,
	16000, //
	16951,
	17959,
	19027,
	20158,
	21357,
	22627,
	23972,
	25398,
	26908,
	28508,
	30203,
	32000,
	33902,
	35918,
	38054,
	40317,
	42714,
	45254,
	47945,
	50796,
	53817,
	57017,
	60407,
	64000
};

// Variables used in Test Strip mode
uint8_t selected_teststrip_exposure_ = 36;
uint8_t displayed_teststrip_exposure_ = selected_teststrip_exposure_;
uint16_t current_teststrip_exposure_ = 0;
uint8_t current_teststrip_exposure_index_ = 0;

// Variables used in Print mode
uint8_t current_print_exposure_ = 0;
uint16_t current_exposure_ms_ = 0;

// Selected resolution
// 1 => 1/12 stops, 2 => 1/6 stops, 4 => 1/3 stops
uint8_t selected_resolution_ = 2;

// 7-segment display
uint8_t current_segment_ = 1;

void setup() {
	// Set up I/O output/input (1 = output)
	DDRC = PC_SEG_MASK;
	DDRD = PD_SEG_MASK | PD_RELAY_CTRL | PD_BUZZER_CTRL;
	DDRB = PB_SEG_MASK;

	// Internal pull-ups on button/switch inputs
	PORTD = PD_BUTTON_IN_MASK;
	
	// Set up timers 1 and 2
	TCCR1A = _BV(WGM12) | _BV(WGM10); // Fast PWM
	TCCR1B = _BV(CS10); // No prescaling
	TCCR2A = _BV(WGM21) | _BV(WGM20); // Fast PWM
	TCCR2B = _BV(CS10); // No prescaling
	
	// Set initial PWM values
	OCR2A = 64;
	OCR1B = 64;
	OCR1A = 64;

	PR_Init();
}

void display_text(const char char1, const char char2, const char char3){
	segment_1_.character = char1;
	segment_1_.dot = false;
	segment_2_.character = char2;
	segment_2_.dot = false;
	segment_3_.character = char3;
	segment_3_.dot = false;
}

void set_character(const char digit, const uint8_t segment, bool dot){
	uint8_t port_c = 0;
	uint8_t port_d = 0;

	switch(digit){
	case 0:
	case 'O':
	case '0':
		port_c = PC_SEG_A | PC_SEG_B | PC_SEG_C | PC_SEG_D | PC_SEG_E | PC_SEG_F;
		break;
	case 1:
	case '1':
		port_c = PC_SEG_B | PC_SEG_C;
		break;
	case 2:
	case '2':
		port_c = PC_SEG_A | PC_SEG_B | PC_SEG_D | PC_SEG_E;
		port_d = PD_SEG_G;
		break;
	case 3:
	case '3':
		port_c = PC_SEG_A | PC_SEG_B | PC_SEG_C | PC_SEG_D;
		port_d = PD_SEG_G;
		break;
	case 4:
	case '4':
		port_c = PC_SEG_B | PC_SEG_C | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 5:
	case '5':
		port_c = PC_SEG_A | PC_SEG_C | PC_SEG_D | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 6:
	case '6':
		port_c = PC_SEG_A | PC_SEG_C | PC_SEG_D | PC_SEG_E | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 7:
	case '7':
		port_c = PC_SEG_A | PC_SEG_B | PC_SEG_C;
		break;
	case 8:
	case '8':
		port_c = PC_SEG_A | PC_SEG_B | PC_SEG_C | PC_SEG_D | PC_SEG_E | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 9:
	case '9':
		port_c = PC_SEG_A | PC_SEG_B | PC_SEG_C | PC_SEG_D | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 'n':
		port_c = PC_SEG_C | PC_SEG_E;
		port_d = PD_SEG_G;
		break;
	case 'E':
		port_c = PC_SEG_A | PC_SEG_D | PC_SEG_E | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 'o':
		port_c = PC_SEG_C | PC_SEG_D | PC_SEG_E;
		port_d = PD_SEG_G;
		break;
	case 'b':
		port_c = PC_SEG_C | PC_SEG_D | PC_SEG_E | PC_SEG_F;
		port_d = PD_SEG_G;
		break;
	case 'd':
		port_c = PC_SEG_C | PC_SEG_D | PC_SEG_E | PC_SEG_B;
		port_d = PD_SEG_G;
		break;
	case ' ':
		port_c = 0;
		break;
	default:
		display_text('E','0','3');
	}

	if(dot){
		port_d |= PD_SEG_DP;
	}

	switch(segment){
	case 1:
		// Enable OC1A, disable OC1B and OC2A
		TCCR1A = (TCCR1A & ~TCCR1A_SEG_MASK) | (TCCR1A_SEG_1 & TCCR1A_SEG_MASK);
		TCCR2A = (TCCR2A & ~TCCR2A_SEG_MASK);
		break;
	case 2:
		// Enable OC1B, disable OC1A and OC2A
		TCCR1A = (TCCR1A & ~TCCR1A_SEG_MASK) | (TCCR1A_SEG_2 & TCCR1A_SEG_MASK);
		TCCR2A = (TCCR2A & ~TCCR2A_SEG_MASK);
		break;
	case 3:
		// Enable OC2A, disable OC1A and OC1B
		TCCR1A = (TCCR1A & ~TCCR1A_SEG_MASK);
		TCCR2A = (TCCR2A & ~TCCR2A_SEG_MASK) | (TCCR2A_SEG_3 & TCCR2A_SEG_MASK);
		break;
	}

	PORTD = (PORTD & ~PD_SEG_MASK) | (port_d & PD_SEG_MASK);
	PORTC = (PORTC & ~PC_SEG_MASK) | (port_c & PC_SEG_MASK);
}


void handle_segment_output(){
	if(current_segment_ == 4){
		current_segment_ = 1;
	}

	if(current_segment_ == 1){
		set_character(segment_1_.character, segment_1_.segment, segment_1_.dot);
	}
	else if(current_segment_ == 2){
		set_character(segment_2_.character, segment_2_.segment, segment_2_.dot);
	}
	else if(current_segment_ == 3){
		set_character(segment_3_.character, segment_3_.segment, segment_3_.dot);
	}
	++current_segment_;
}

void display_ms(uint16_t milliseconds){
	// Display milliseconds as seconds with 1 decimal
	uint16_t digit_1 = milliseconds/10000;
	uint16_t digit_2 = (milliseconds - digit_1*10000)/1000;
	uint16_t digit_3 = (milliseconds - digit_1*10000 - digit_2*1000)/100;
	segment_1_.character = digit_1;
	segment_1_.dot = false;
	segment_2_.character = digit_2;
	segment_2_.dot = true;
	segment_3_.character = digit_3;
	segment_3_.dot = false;
}

void display_int(uint8_t integer){
	uint16_t digit_1 = integer/100;
	uint16_t digit_2 = (integer - digit_1*100)/10;
	uint16_t digit_3 = (integer - digit_1*100 - digit_2*10);
	segment_1_.character = digit_1;
	segment_1_.dot = false;
	segment_2_.character = digit_2;
	segment_2_.dot = false;
	segment_3_.character = digit_3;
	segment_3_.dot = false;
}

void check_buttons(){
	DDRB = (DDRB & ~PB_BUTTON_V_MASK) | (PB_BUTTON_V_1 & PB_BUTTON_V_MASK);
	delayMicroseconds(100);
	
	// button START
	if (!(PIND & PD_BUTTON_IN_1)) {
		if (buttons_debounce_.start_low_count == N_DEBOUNCE_ITERS) {
			buttons_.start_pressed = true;
		} else {
			buttons_debounce_.start_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.start_low_count == 0) {
			buttons_.start_pressed = false;
		} else if (buttons_debounce_.start_low_count > 0) {
			buttons_debounce_.start_low_count -= 1;
		}
	}
	
	// switch CHANNEL
	if (!(PIND & PD_BUTTON_IN_2)) {
		if (buttons_debounce_.channel_low_count == N_DEBOUNCE_ITERS) {
			buttons_.channel_active = true;
		} else {
			buttons_debounce_.channel_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.channel_low_count == 0) {
			buttons_.channel_active = false;
		} else if (buttons_debounce_.channel_low_count > 0) {
			buttons_debounce_.channel_low_count -= 1;
		}
	}
	
	// switch OVERRIDE
	if (!(PIND & PD_BUTTON_IN_3)) {
		if (buttons_debounce_.override_low_count == N_DEBOUNCE_ITERS) {
			buttons_.override_active = true;
		} else {
			buttons_debounce_.override_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.override_low_count == 0) {
			buttons_.override_active = false;
		} else if (buttons_debounce_.override_low_count > 0) {
			buttons_debounce_.override_low_count -= 1;
		}
	}
	
	// switch MODE
	if (!(PIND & PD_BUTTON_IN_4)) {
		if (buttons_debounce_.mode_low_count == N_DEBOUNCE_ITERS) {
			buttons_.mode_active = true;
		} else {
			buttons_debounce_.mode_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.mode_low_count == 0) {
			buttons_.mode_active = false;
		} else if (buttons_debounce_.mode_low_count > 0) {
			buttons_debounce_.mode_low_count -= 1;
		}
	}
	
	DDRB = (DDRB  & ~PB_BUTTON_V_MASK) | (PB_BUTTON_V_2 & PB_BUTTON_V_MASK);
	delayMicroseconds(100);
	
	// switch STOPS ONE ONETH
	if (!(PIND & PD_BUTTON_IN_1)) {
		if (buttons_debounce_.stops_onesixth_low_count == N_DEBOUNCE_ITERS) {
			buttons_.stops_onesixth_active = true;
		} else {
			buttons_debounce_.stops_onesixth_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.stops_onesixth_low_count == 0) {
			buttons_.stops_onesixth_active = false;
		} else if (buttons_debounce_.stops_onesixth_low_count > 0) {
			buttons_debounce_.stops_onesixth_low_count -= 1;
		}
	}

	// switch STOPS ONE SIXTH
	if (!(PIND & PD_BUTTON_IN_2)) {
		if (buttons_debounce_.stops_oneoneth_low_count == N_DEBOUNCE_ITERS) {
			buttons_.stops_oneoneth_active = true;
		} else {
			buttons_debounce_.stops_oneoneth_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.stops_oneoneth_low_count == 0) {
			buttons_.stops_oneoneth_active = false;
		} else if (buttons_debounce_.stops_oneoneth_low_count > 0) {
			buttons_debounce_.stops_oneoneth_low_count -= 1;
		}
	}
	
	DDRB = (DDRB  & ~PB_BUTTON_V_MASK) | (PB_BUTTON_V_3 & PB_BUTTON_V_MASK);
	delayMicroseconds(100);

	// button PLUS
	if (!(PIND & PD_BUTTON_IN_1)) {
		if (buttons_debounce_.plus_low_count == N_DEBOUNCE_ITERS) {
			buttons_.plus_pressed = true;
		} else {
			buttons_debounce_.plus_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.plus_low_count == 0) {
			buttons_.plus_pressed = false;
		} else if (buttons_debounce_.plus_low_count > 0) {
			buttons_debounce_.plus_low_count -= 1;
		}
	}

	// button MINUS
	if (!(PIND & PD_BUTTON_IN_2)) {
		if (buttons_debounce_.minus_low_count == N_DEBOUNCE_ITERS) {
			buttons_.minus_pressed = true;
		} else {
			buttons_debounce_.minus_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.minus_low_count == 0) {
			buttons_.minus_pressed = false;
		} else if (buttons_debounce_.minus_low_count > 0) {
			buttons_debounce_.minus_low_count -= 1;
		}
	}

	// button DOWN
	if (!(PIND & PD_BUTTON_IN_3)) {
		if (buttons_debounce_.down_low_count == N_DEBOUNCE_ITERS) {
			buttons_.down_pressed = true;
		} else {
			buttons_debounce_.down_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.down_low_count == 0) {
			buttons_.down_pressed = false;
		} else if (buttons_debounce_.down_low_count > 0) {
			buttons_debounce_.down_low_count -= 1;
		}
	}
	
	// button UP
	if (!(PIND & PD_BUTTON_IN_4)) {
		if (buttons_debounce_.up_low_count == N_DEBOUNCE_ITERS) {
			buttons_.up_pressed = true;
		} else {
			buttons_debounce_.up_low_count += 1;
		}
	}
	else {
		if (buttons_debounce_.up_low_count == 0) {
			buttons_.up_pressed = false;
		} else if (buttons_debounce_.up_low_count > 0) {
			buttons_debounce_.up_low_count -= 1;
		}
	}
	
	DDRB = (DDRB & ~PB_BUTTON_V_MASK);
}


void increment_selected_teststrip_exposure(){
	if (selected_teststrip_exposure_
			< kNExposures - selected_resolution_*N_TEST_STRIPS) {
		selected_teststrip_exposure_ += selected_resolution_;
	}
}

void decrement_selected_teststrip_exposure(){
	if (selected_teststrip_exposure_ > selected_resolution_) {
		selected_teststrip_exposure_ -= selected_resolution_;
	} else {
		selected_teststrip_exposure_ = 0;
	}
}

void increment_displayed_teststrip_exposure(){
	if (displayed_teststrip_exposure_ < selected_teststrip_exposure_
			+ (N_TEST_STRIPS - 1)*selected_resolution_) {
		displayed_teststrip_exposure_ += selected_resolution_;
	} else {
		displayed_teststrip_exposure_ = selected_teststrip_exposure_;
	}
}

void decrement_displayed_teststrip_exposure(){
	if (displayed_teststrip_exposure_
			>= selected_teststrip_exposure_ + selected_resolution_) {
		displayed_teststrip_exposure_ -= selected_resolution_;
	} else {
		displayed_teststrip_exposure_ = selected_teststrip_exposure_
			+ (N_TEST_STRIPS - 1)*selected_resolution_;
	}
}



void start_exposure(){
	if (RELAY_ENABLED) {
		PORTD |= PD_RELAY_CTRL;
	}
}


void stop_exposure(){
	if (RELAY_ENABLED) {
		PORTD &= ~PD_RELAY_CTRL;
	}
}


void start_buzzer(){
	if (BUZZER_ENABLED) {
		PORTD |= PD_BUZZER_CTRL;
	}
}


void stop_buzzer(){
	if (BUZZER_ENABLED) {
		PORTD &= ~PD_BUZZER_CTRL;
	}
}

State state_advance(){
	State next_state = state_;
	
	switch(state_){
	case kInit:
		if(millis() - event_start_ms_ > START_DELAY){
			next_state = kTestStripIdle;
		}
		break;
	case kTestStripIdle: {
		if (buttons_.plus_pressed) {
			next_state = kTestStripIncExp;
		} else if (buttons_.minus_pressed) {
			next_state = kTestStripDecExp;
		} else if (buttons_.up_pressed) {
			next_state = kTestStripIncDisp;
		} else if (buttons_.down_pressed) {
			next_state = kTestStripDecDisp;
		} else if (buttons_.start_pressed) {
			next_state = kTestStripStart;
		} else if (buttons_.mode_active) {
			next_state = kPrintIdle;
		} else if (buttons_.override_active) {
			next_state = kTestStripOverrideOn;
		}
	} break;
	case kTestStripOverrideOn:
		if (!buttons_.override_active) {
			next_state = kPrintIdle;
		}
		break;
	case kTestStripIncExp:
		if (!buttons_.plus_pressed){
			next_state = kTestStripIdle;
		}
		break;
	case kTestStripDecExp:
		if (!buttons_.minus_pressed){
			next_state = kTestStripIdle;
		}
		break;
	case kTestStripIncDisp:
		if (!buttons_.up_pressed){
			next_state = kTestStripIdle;
		}
		break;
	case kTestStripDecDisp:
		if (!buttons_.down_pressed){
			next_state = kTestStripIdle;
		}
		break;
	case kTestStripStart:
		if (!buttons_.start_pressed) {
			next_state = kTestStripDelay;
		}
		break;
	case kTestStripDelay:
		if (buttons_.start_pressed) {
			next_state = kTestStripCancel;
		}	else if (millis() - event_start_ms_ > START_DELAY) {
			next_state = kTestStripExp;
		}
		break;
	case kTestStripExp:
		if (buttons_.start_pressed) {
			next_state = kTestStripCancel;
		}	else if (millis() - event_start_ms_ > current_teststrip_exposure_) {
			if (current_teststrip_exposure_index_ == N_TEST_STRIPS - 1) {
				next_state = kTestStripIdle;
			} else {
				current_teststrip_exposure_
					= kExposures[selected_teststrip_exposure_
											 + current_teststrip_exposure_index_*selected_resolution_
											 + selected_resolution_]
					- kExposures[selected_teststrip_exposure_
											 + current_teststrip_exposure_index_*selected_resolution_
											 ];
				++current_teststrip_exposure_index_;
				next_state = kTestStripDelay;
			}
		}
		break;
	case kTestStripCancel:
		if (!buttons_.start_pressed) {
			next_state = kTestStripIdle;
		}
		break;
	case kPrintIdle:
		if (buttons_.plus_pressed) {
			next_state = kPrintIncExp;
		} else if (buttons_.minus_pressed) {
			next_state = kPrintDecExp;
		} else if (buttons_.up_pressed) {
			next_state = kPrintUpPressed;
		} else if (buttons_.down_pressed) {
			next_state = kPrintDownPressed;
		} else if (buttons_.start_pressed) {
			next_state = kPrintStart;
		} else if (!buttons_.mode_active) {
			next_state = kTestStripIdle;
		} else if (buttons_.override_active) {
			next_state = kTestStripOverrideOn;
		}
		break;
	case kPrintOverrideOn:
		if (!buttons_.override_active) {
			next_state = kPrintIdle;
		}
		break;
	case kPrintIncExp:
		if (!buttons_.plus_pressed){
			next_state = kPrintIdle;
		}
		break;
	case kPrintDecExp:
		if (!buttons_.minus_pressed){
			next_state = kPrintIdle;
		}
		break;
	case kPrintUpPressed:
		 if (!buttons_.up_pressed) {
			next_state = kPrintDispSetting;
		} 
		break;
	case kPrintDownPressed:
		 if (!buttons_.down_pressed) {
			next_state = kPrintDispSetting;
		} 
		break;
	case kPrintDispSetting:
		if (buttons_.up_pressed) {
			next_state = kPrintIncSetting;
		} else if (buttons_.down_pressed) {
			next_state = kPrintDecSetting;
		} else if ((millis() - event_start_ms_ > START_DELAY)
							 || buttons_.plus_pressed
							 || buttons_.minus_pressed) {
			next_state = kPrintIdle;
		}
		break;
	case kPrintIncSetting:
		if (!buttons_.up_pressed) {
			next_state = kPrintDispSetting;
		} 
		break;
	case kPrintDecSetting:
		if (!buttons_.down_pressed) {
			next_state = kPrintDispSetting;
		} 
		break;
	case kPrintStart:
		if (!buttons_.start_pressed) {
			next_state = kPrintDelay;
		}
		break;
	case kPrintDelay:
		if (buttons_.start_pressed) {
			next_state = kPrintCancel;
		}	else if (millis() - event_start_ms_ > START_DELAY) {
			next_state = kPrintExposure;
		}
		break;
	case kPrintExposure:
		if (buttons_.start_pressed) {
			next_state = kPrintCancel;
		}	else if (millis() - event_start_ms_ >= current_exposure_ms_ ) {
			if (PR_is_last_exposure(current_print_exposure_)) {
				next_state = kPrintIdle;
			} else {
				// Continue with the next exposure
				next_state = kPrintDelay;
			}
		}
		break;
	case kPrintCancel:
		if (!buttons_.start_pressed) {
			next_state = kPrintIdle;
		}	
		break;
	default:
		display_text('E','0','0');
		next_state = kError;
	}
	
	return next_state;
}

void state_loop_tasks(){
	switch(state_){
	case kTestStripOverrideOn:
	case kPrintOverrideOn:
		display_ms((millis() - event_start_ms_) % 60000);
		break;
	case kTestStripExp:
		display_ms(current_teststrip_exposure_ - (millis() - event_start_ms_));
		break;
	case kPrintIdle:
		if (buttons_.channel_active && PR_get_channel() == 0) {
			PR_set_channel(1);
			PR_display_current_setting_value();
		} else if (!buttons_.channel_active && PR_get_channel() == 1) {
			PR_set_channel(0);
			PR_display_current_setting_value();
		}
		break;
	case kPrintExposure:
		display_ms(current_exposure_ms_ - (millis() - event_start_ms_));
		break;
	case kTestStripDelay:
	case kPrintDelay:
		// Buzzer on at three 100 ms intervals
		if (millis() - event_start_ms_ > 3*START_DELAY/4 + 100) {
			stop_buzzer();
		} else if (millis() - event_start_ms_ > 3*START_DELAY/4) {
			start_buzzer();
		} else if (millis() - event_start_ms_ > 2*START_DELAY/4 + 100) {
			stop_buzzer();
		} else if (millis() - event_start_ms_ > START_DELAY/4) {
			start_buzzer();
		} else if (millis() - event_start_ms_ > START_DELAY/4 + 100) {
			stop_buzzer();
		} else if (millis() - event_start_ms_ > START_DELAY/4) {
			start_buzzer();
		}
		break;
	}
}

void state_enter_tasks(){
	switch(state_){
	case kTestStripIdle:
		display_ms(kExposures[displayed_teststrip_exposure_]);
		current_teststrip_exposure_index_ = 0;
		current_teststrip_exposure_ = kExposures[selected_teststrip_exposure_];
		break;
	case kTestStripOverrideOn:
	case kPrintOverrideOn:
		event_start_ms_ = millis();
		start_exposure();
		break;
	case kTestStripIncExp:
		increment_selected_teststrip_exposure();
		display_ms(kExposures[selected_teststrip_exposure_]);
		displayed_teststrip_exposure_ = selected_teststrip_exposure_;
		break;
	case kTestStripDecExp:
		decrement_selected_teststrip_exposure();
		display_ms(kExposures[selected_teststrip_exposure_]);
		displayed_teststrip_exposure_ = selected_teststrip_exposure_;
		break;
	case kTestStripIncDisp:
		increment_displayed_teststrip_exposure();
		display_ms(kExposures[displayed_teststrip_exposure_]);
		break;
	case kTestStripDecDisp:
		decrement_displayed_teststrip_exposure();
		display_ms(kExposures[displayed_teststrip_exposure_]);
		break;
	case kTestStripDelay:
		display_text(current_teststrip_exposure_index_ + 1, 'o', N_TEST_STRIPS);
		event_start_ms_ = millis();
		break;
	case kTestStripExp:
		event_start_ms_ = millis();
		start_exposure();
		break;
	case kPrintExposure:
		event_start_ms_ = millis();
		current_exposure_ms_ = PR_exposure_ms(current_print_exposure_);
		start_exposure();
		break;
	case kPrintIdle:
		PR_display_current_setting_value();
		break;
	case kPrintIncExp:
		PR_increment_setting_value();
		PR_display_current_setting_value();
		break;
	case kPrintDecExp:
		PR_decrement_setting_value();
		PR_display_current_setting_value();
		break;
	case kPrintDispSetting:
		PR_display_current_setting_name();
		event_start_ms_ = millis();
		break;
	case kPrintIncSetting:
		PR_increment_setting_index();
		PR_display_current_setting_name();
		break;
	case kPrintDecSetting:
		PR_decrement_setting_index();
		PR_display_current_setting_name();
		break;
	case kPrintDelay:
		PR_display_setting_name(current_print_exposure_);
		event_start_ms_ = millis();
		break;
	case kPrintStart:
		current_print_exposure_ = PR_get_first_print_exposure_index();
		break;
	}
}

void state_exit_tasks(){
	switch(state_){
	case kTestStripOverrideOn:
	case kPrintOverrideOn:
	case kTestStripExp:
	case kPrintExposure:
		++current_print_exposure_;
		stop_exposure();
		break;
	case kTestStripDelay:
	case kPrintDelay:
		stop_buzzer();
		break;
	}
}

void loop() {
	handle_segment_output();
	check_buttons();

	if (buttons_.stops_oneoneth_active) {
		selected_resolution_ = 4;
	} else if (buttons_.stops_onesixth_active) {
		selected_resolution_ = 1;
	} else {
		selected_resolution_ = 2;
	}
	
	State next_state = state_advance();
	if(state_ != next_state){
		state_exit_tasks();
		state_ = next_state;
		state_enter_tasks();
	}
	state_loop_tasks();
	
	delay(1);
}
