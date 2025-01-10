/* Copyright 2022 Columbia University, SLD Group */

#ifndef __ACCELERATED_SIM_H__
#define __ACCELERATED_SIM_H__


//------------------------------------------
#if defined(SMALL)
			#define REDUCED_CHANS(_l)			\
				(					\
					(_l ==  6) ?    10 :		\
					(_l ==  5) ?    64 :		\
					(_l ==  4) ?    8 :		\
					(_l ==  3) ?    16 :		\
					(_l ==  2) ?    4 :		\
					3				\
				)
			//4 32
			#define REDUCED_CHANS_NATIVE(_l)		\
				(					\
					(_l ==  6) ?    10 :		\
					(_l ==  5) ?    64 :		\
					(_l ==  4) ?    8 :		\
					(_l ==  3) ?    16 :		\
					(_l ==  2) ?    4 :		\
					3				\
				)          
 #elif defined(MEDIUM)
			#define REDUCED_CHANS(_l)			\
				(					\
					(_l ==  6) ?    10 :		\
					(_l ==  5) ?    64 :		\
					(_l ==  4) ?    8 :		\
					(_l ==  3) ?    16 :		\
					(_l ==  2) ?    4 :		\
					3				\
				)
			//4 32
			#define REDUCED_CHANS_NATIVE(_l)		\
				(					\
					(_l ==  6) ?    10 :		\
					(_l ==  5) ?    64 :		\
					(_l ==  4) ?    8 :		\
					(_l ==  3) ?    16 :		\
					(_l ==  2) ?    4 :		\
					3				\
				)         
 #elif defined(FAST)
			#define REDUCED_CHANS(_l)			\
				(					\
					(_l ==  6) ?    10 :		\
					(_l ==  5) ?    64 :		\
					(_l ==  4) ?    4 :		\
					(_l ==  3) ?    16 :		\
					(_l ==  2) ?    4 :		\
					3				\
				)
			//4 32
			#define REDUCED_CHANS_NATIVE(_l)		\
				(					\
					(_l ==  6) ?    10 :		\
					(_l ==  5) ?    64 :		\
					(_l ==  4) ?    4 :		\
					(_l ==  3) ?    16 :		\
					(_l ==  2) ?    4 :		\
					3				\
				)       
 #endif 

// #define REDUCED_CHANS(_l)			\
// 	(					\
// 		(_l ==  6) ?    10 :		\
// 		(_l ==  5) ?    64 :		\
// 		(_l ==  4) ?    4 :		\
// 		(_l ==  3) ?    16 :		\
// 		(_l ==  2) ?    4 :		\
// 		3				\
// 	)
// //4 32
// #define REDUCED_CHANS_NATIVE(_l)		\
// 	(					\
// 		(_l ==  6) ?    10 :		\
// 		(_l ==  5) ?    64 :		\
// 		(_l ==  4) ?    4 :		\
// 		(_l ==  3) ?    16 :		\
// 		(_l ==  2) ?    4 :		\
// 		3				\
// 	)

// ------------------------------------------------------


#endif // __ACCELERATED_SIM_H__


