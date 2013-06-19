/* arithmetic.cc
 * Madeleine Clute (mclute@andrew / madeleine.clute@gmail.com)
 * June 2013
 */

 /* TODO: DIFFICULTY LEVEL ENTRY AS WELL */

#include "arithmetic.h"
#include "Dots.h"
#include <math.h>


int math_mode;/* default for now */
static int choose_mode = 1;  /* mode needs to be chose first */
static int choose_difficulty = 1; /* can selct difficutly and move up */
static int digit_position = 0;
static int num_digits = 0;
static int difficulty_level = 2; /* how many digits the answer can be */
static int number_sign = 1; /* need to to type the # sign */
/* TODO: figure out if difficulty should be changable from menu or
as students get more advanced */

/* experiment to block the backlog of key presses */

static time_t last_pressed_time = time(0);
static int last_button = 0; //default

 Arithmetic::Arithmetic(IOEventParser& my_iep, 
 		const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  		IBTApp(my_iep, path_to_mapping_file), su(my_su), math_s("./resources/Voice/math_sounds/"), 
  		target_sequence('\0'), current_sequence('\0'), nomirror(f), dots1('\0'), dots2('\0')
{
  //su->saySound(math_s, "arithmetic_practice");
  printf("starting practice\n");
  srand(time(0)); // initializing the random seed
  Fact_new();  
}
  

Arithmetic::~Arithmetic()
{
  delete su;
}


void Arithmetic::processEvent(IOEvent& e)
{
	//std::cout << "processEvent" << std::endl;
  if( e.type == IOEvent::BUTTON_DOWN && e.button == 0 && choose_mode == 0){
    //choose_mode = 1; // for next time
    sayArithmeticQuestion(false); // repeat question
    return;
  }
  if (e.type == IOEvent::BUTTON_DOWN && choose_mode == 1) {
    /* need to interpret it as mode switching */
    printf("dot is %d\n", e.button);
    if (e.button == 4) {
      math_mode = ADDITION;
      choose_mode = 0;
      Fact_new();
    }
    else if (e.button == 5) {
      math_mode = SUBTRACTION;
      choose_mode = 0;
      Fact_new();
    }
    else if (e.button == 6) {
      math_mode = MULTIPLICATION;
      choose_mode = 0;
      Fact_new();
    }
    else {
        printf("malformed input, please select buttons 1, 2, or 3\n");
    }
    /* turn it off and "restart" the activity */
    
  }
  else if( e.type == IOEvent::STYLUS_DOWN || 
          e.type == IOEvent::BUTTON_DOWN  && 
          ((time(0) - last_pressed_time) >= 1))
  {
    last_pressed_time = time(0);
    AP_attempt(getDot(e));
    
  }
  else if(time(0) == last_pressed_time && e.button == last_button){ // most likely a jammed button
    printf("caught something\n");
  }
}

void Arithmetic::Fact_new()
{
  clearArray(num1_array);
  clearArray(response_array);
  clearArray(num2_array);
  /* this is the default behavior of fact_new */
  if (!choose_mode) {
    printf("called fact new\n");
    //generate a random number between 0 to 9
    int result = -1; // ensures too small to start
    int num1, num2;
    int max = pow(10, difficulty_level);
    while(result >= max || result < 0){
      
    	num1 = (int) rand() % max; 
    	num2 = (int) rand() % max;
      if (math_mode == ADDITION) {
    	   result = num1 + num2;
      }
      else if (math_mode == SUBTRACTION) {
         result = num1 - num2;
      }
      else if (math_mode == MULTIPLICATION) {
         result = num1 * num2;
      }
    }
    current_sequence = '\0';
    
    dots1 = convertToDotSequence(IBTApp::getCurrentCharset(), num1);
    dots2 = convertToDotSequence(IBTApp::getCurrentCharset(), num2);
    std::cout<<"question is "<<num1 << " and " << num2 <<std::endl;
    std::cout<<"desired result is:"<<result<<std::endl;
    printf("%d\n",result);
    /* populate the digit array with the digits from the result and nums */
    getDigits(result, response_array);
    getDigits(num1, num1_array);
    getDigits(num2, num2_array);
    num_digits = (result == 0) ? (1) : ((int) log10(result) + 1);
    sayArithmeticQuestion(false); // don't say answer yet
  }

  /* this is not default behavior and is to allow the user to select modes */
  else {
    /* TODO: have SL record things for this explaining */
    su->saySound(math_s, "maths_instructions");
    printf("please enter math mode:\n \
            press button 1 for addition, \n \
            press button 2 for subtraction, \n \
            press button 3 for multiplication\n");
  }

}

void Arithmetic::clearArray(int *a)
{
  int i;
  for (i = 0; i < MAX_DIGITS; i++){
    a[i] = -1;
  }
}
void Arithmetic::getDigits(int num, int *digit_array)
{
  int digit, i;
  int len = (num == 0) ? (1) : ((int) log10(num) + 1); // local use
  printf("num digits %d", len);
  for (i = 0; i < len; i++) {
    digit = num % 10;
    num /= 10;
    digit_array[MAX_DIGITS - i - 1] = digit;
    printf(" digit %d", digit);
  }
  printf("\n");
}

void Arithmetic::sayArithmeticQuestion(bool say_answer)
{
  
  std::cout << "sayArithmeticQuestion" << std::endl;
  static const Charset& charset = IBTApp::getCurrentCharset();
  if (!say_answer) {
    su->saySound(math_s, "please_write_the_number_that_is_equal_to");
  }
  /* sandwich the sounds together */
  say_multidigit(num1_array); // say first number
 
  if (math_mode == ADDITION) {
  	 su->saySound(math_s, "plus");
  }
  else if (math_mode == SUBTRACTION) {
  	 su->saySound(math_s, "minus");
  }
  else if (math_mode == MULTIPLICATION) {
  	 su->saySound(math_s, "into");
  }
  else if (math_mode == DIVISION) {
  	 su->saySound(math_s, "divided_by");
  }
  
  say_multidigit(num2_array); // say second number
  
  if (say_answer){
      su->saySound(math_s, "equals");
      say_multidigit(response_array);
  }
  
}

/* Splits the number into digits and says it by digit */
/* need to have global arrays that are passed by reference to 
 * the get digit function */ 

/* TODO: need to be able to handle numbers like 117 */
void Arithmetic::say_multidigit(int *a)
{
  int i, n;
  bool multidigit = false; // to know to say 0 if it on its own
  char buf[50]; // 
  for (i = 0; i < MAX_DIGITS; i++){
    if (a[i] != -1 && (a[i] != 0 || !multidigit)){ /* checking for how to say #'s */
      switch (MAX_DIGITS - i) {
        case 1: 
          printf("ones place %d\n", a[i]);
          sprintf(buf,"sl_%d", a[i]);
          su->saySound(math_s, buf);
          break;
        case 2:
          printf("tens place %d\n", a[i]);
          multidigit = true;
          if (a[i] == 1) {
            /* then special case */
            n = 10 + a[i+1]; // get next digit
            sprintf(buf, "sl_%d", n);
            su->saySound(math_s, buf);
            return;
          }
          else{
            multidigit = true;
            sprintf(buf,"sl_10_%d",a[i]);
            su->saySound(math_s, buf);
          } 
          break;
        case 3: 
          printf("hundreds place %d\n", a[i]);
          sprintf(buf,"sl_%d", a[i]);
          su->saySound(math_s, buf);
          su->saySound(math_s, "hundred");
          break;
        default:
          break;
      }
      
    }
  }
}

void Arithmetic::AP_attempt(unsigned char dot)
{

	su->sayNumber(getStudentVoice(), dot, nomirror);
  printf("got here\n");
  int strt = MAX_DIGITS - num_digits; // where to start in array
  i = 0;
  printf("digit pos %d\n", digit_position);

  current_target = response_array[strt + digit_position];

  printf("current target %d", current_target);
  if (!number_sign){
    target_sequence = convertToDotSequence(IBTApp::getCurrentCharset(),
                                          current_target);
  }
  else {
    target_sequence = MATH_FLAG;
  }
  //Check if user hit the right dot (ie, the dot exists in the target sequence)
  printf("target sequence is %d\n", target_sequence);
  if( my_dot_mask(dot) & target_sequence )
  {
    current_sequence = current_sequence | my_dot_mask(dot); //add the dot to the current on-going sequence
    //are we done?
    if( current_sequence == target_sequence )
    {
      
      if ((digit_position >= num_digits - 1) && !number_sign){ //hanve reached the end of the #
        printf("digits %d num digits %d\n", digit_position, num_digits);
        digit_position = 0; // reset it
        su->saySound(getTeacherVoice(), "tada"); // plays a nice sound
        sayArithmeticQuestion(true);
        number_sign = 1; // set it back for next time
        Fact_new(); /* ideally repeat the question and answer here */
      } 
      else {
        // TODO: add capabilities to say number
      
        su->saySound(math_s, "good_next"); //asks for next digit
        if (number_sign == 0){
          digit_position++;
          printf("incrememting\n");
        }
        current_sequence = 0;
        number_sign = !number_sign;
        printf("number sign is no %d\n", number_sign);
        
      }
      return;
    }
  }
  else
  {
    su->saySound(getTeacherVoice(), "no");
   // sayArithmeticQuestion(num1,num2); TODO inmplement ques repeat
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishArithmeticPractice::EnglishArithmeticPractice(IOEventParser& my_iep) :
  Arithmetic(my_iep, "./language_mapping_files/number_mapping.txt", 
  			   new EnglishSoundsUtil, false)
{

}