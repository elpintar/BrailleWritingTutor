/* arithmetic.cc
 * Madeleine Clute (mclute@andrew / madeleine.clute@gmail.com)
 * June 2013
 */

 /* TODO: DIFFICULTY LEVEL ENTRY AS WELL */

#include "arithmetic.h"
#include "Dots.h"
#include <math.h>

#define MAX_DIGITS 4 /* maximum digits that an answer can have */

int math_mode;/* default for now */
static int choose_mode = 1;  /* mode needs to be chose first */
static int digit_position = 0;
static int digit_array[MAX_DIGITS];
static int num_digits = 0;
static int difficulty_level = 2; /* how many digits the answer can be */



 Arithmetic::Arithmetic(IOEventParser& my_iep, 
 		const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  		IBTApp(my_iep, path_to_mapping_file), su(my_su), math_s("./resources/Voice/math_sounds/"), 
  		target_sequence('\0'), current_sequence('\0'), nomirror(f), dots1('\0'), dots2('\0')
{
  //su->saySound(math_s, "arithmetic_practice");
  printf("starting practice\n");
  Fact_new();  
}
  

Arithmetic::~Arithmetic()
{
  delete su;
}


void Arithmetic::processEvent(IOEvent& e)
{
	//std::cout << "processEvent" << std::endl;
  if( e.type == IOEvent::BUTTON_DOWN && e.button == 0 )
    return;
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
  else if( e.type == IOEvent::STYLUS_DOWN || e.type == IOEvent::BUTTON_DOWN )
  {
    AP_attempt(getDot(e));
  }
}

void Arithmetic::Fact_new()
{
  /* this is the default behavior of fact_new */
  if (!choose_mode) {
    printf("called fact new\n");
    //generate a random number between 0 to 9
    int result = -1; // ensures too small to start
    int num1, num2;
    int max = pow(10, difficulty_level);
    while(result >= max || result < 0){
      printf("result %d mode %d\n", result, math_mode);
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
    /* populate the digit array with the digits from the result */
    getDigits(result);
    sayArithmeticQuestion(dots1, dots2);
  }

  /* this is not default behavior and is to allow the user to select modes */
  else {
    /* TODO: have SL record things for this explaining */
    
    printf("please enter math mode:\n \
            press button 1 for addition, \n \ 
            press button 2 for subtraction, \n \
            press button 3 for multiplication\n");
  }

}

void Arithmetic::getDigits(int num) 
{
  int digit, i;
  num_digits = (num == 0) ? (1) : ((int) log10(num) + 1);
  printf("num digits %d", num_digits);
  for (i = 0; i < num_digits; i++) {
    digit = num % 10;
    num /= 10;
    digit_array[MAX_DIGITS - i - 1] = digit;
    printf(" digit %d", digit);
  }
  printf("\n");
}

void Arithmetic::sayArithmeticQuestion(const DotSequence& d1, 
									   const DotSequence& d2) const
{
  std::cout << "sayArithmeticQuestion" << std::endl;
  
  static const Charset& charset = IBTApp::getCurrentCharset();
  int n1,n2;
  n1 = atoi(((std::string) charset[d1]).c_str()); //get the number
  n2 = atoi(((std::string) charset[d2]).c_str());
  su->saySound(math_s, "please_write_the_number_that_is_equal_to");
  /* sandwich the sounds together */


  /* TODO: put in looping here */
  su->sayNumber(getTeacherVoice(), n1, false); //flip is false regardless of mirrored or unmirrored
  
  if (math_mode == ADDITION) {
  	 su->saySound(getTeacherVoice(), "plus");
  }
  else if (math_mode == SUBTRACTION) {
  	 su->saySound(getTeacherVoice(), "minus");
  }
  else if (math_mode == MULTIPLICATION) {
  	 su->saySound(getTeacherVoice(), "times");
  }
  else if (math_mode == DIVISION) {
  	 su->saySound(getTeacherVoice(), "divided_by");
  }
  su->sayNumber(getTeacherVoice(), n2, false);
}

void Arithmetic::AP_attempt(unsigned char dot)
{
	su->sayNumber(getStudentVoice(), dot, nomirror);
  printf("got here\n");
  static int strt = MAX_DIGITS - num_digits; // where to start in array
  i = 0;
  for (i = 0; i<MAX_DIGITS; i++){

    printf("index %d is %d\n",i, digit_array[i]);
  }
  printf("digit pos %d\n", digit_position);
  current_target = digit_array[strt + digit_position];
  printf("current target %d", current_target);
  target_sequence = convertToDotSequence(IBTApp::getCurrentCharset(),
                                         current_target);
  //Check if user hit the right dot (ie, the dot exists in the target sequence)
  
  if( my_dot_mask(dot) & target_sequence )
  {
    current_sequence = current_sequence | my_dot_mask(dot); //add the dot to the current on-going sequence
    //are we done?
    if( current_sequence == target_sequence )
    {
      su->saySound(getTeacherVoice(), "good");
      if (digit_position >= num_digits - 1){ //hanve reached the end of the #
        printf("digits %d num digits %d\n", digit_position, num_digits);
        digit_position = 0; // reset it
        Fact_new(); /* ideally repeat the question and answer here */
      } 
      else {
        digit_position++;
        current_sequence = 0;
        printf("incrememting\n");
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