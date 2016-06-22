//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//

#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "";
const char *studentID   = "";
const char *email       = "";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[5] = { "Static", "Gshare", "Local",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
#define SSG 0 // strongly select the global predictor
#define WSG 1 // weakly select the global predictor
#define WSL 2 // weakly select the local predictor
#define SSL 3 // strongly select the local predictor

#define POWER(x) (1<<x) // calculate 2^x

#define MAXVALUE (POWER(6)-1)
#define MINVALUE (-1*POWER(6))

#define cus_ghistoryBits 31
#define cus_pcIndexBits 12

uint32_t ghr;
uint32_t bhr;

uint32_t *pht;
uint8_t *gbht;
uint8_t *lbht;
uint8_t *choicer;
int **percep_tab;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

int mask(int x)
{
	// generate x bit '1'
	x = (1<<x);
	x--;
	return x;
}

uint8_t check_threshold(int y)
{
	// thereshold function for perceptron predictor
	int threshold = (int)1.93*ghistoryBits + 14;
	if((y > threshold) || (y < (-1 * threshold)))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int set_limit(int w)
{
	// limit the range of weight for perceptron predictor
	if(w > MAXVALUE)
	{
		w = MAXVALUE;
	}
	
	if(w < MINVALUE)
	{
		w = MINVALUE;
	}
	
	return w;
}

//------------------------------------//
//        Predictor Initialization    //
//------------------------------------//
void init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
	case GSHARE:
	  {
		  gbht = (uint8_t*)malloc(sizeof(uint8_t)*POWER(ghistoryBits));
		  ghr = 0;

		  for(int i = 0; i < POWER(ghistoryBits); i++)
		  {
			gbht[i] = NOTTAKEN;
		  }
	  }
    case LOCAL:
	  {
		  bhr = 0;
		  pht = (uint32_t*)malloc(sizeof(uint32_t)*POWER(pcIndexBits));
		  lbht = (uint8_t*)malloc(sizeof(uint8_t)*POWER(lhistoryBits));

		  for(int i = 0; i < POWER(pcIndexBits); i++)
		  {
			pht[i] = NOTTAKEN;
		  }

		  for(int i = 0; i < POWER(lhistoryBits); i++)
		  {
			lbht[i] = NOTTAKEN;
		  }
	  }
    case TOURNAMENT:
	  {
		  ghr = 0;
		  bhr = 0;

		  pht = (uint32_t*)malloc(sizeof(uint32_t)*POWER(pcIndexBits));
		  lbht = (uint8_t*)malloc(sizeof(uint8_t)*POWER(lhistoryBits));
		  gbht = (uint8_t*)malloc(sizeof(uint8_t)*POWER(ghistoryBits));
		  choicer = (uint8_t*)malloc(sizeof(uint8_t)*POWER(ghistoryBits));

		  for(int i = 0; i < POWER(pcIndexBits); i++)
		  {
			pht[i] = NOTTAKEN;
		  }

		  for(int i = 0; i < POWER(lhistoryBits); i++)
		  {
			lbht[i] = NOTTAKEN;
		  }

		  for(int i = 0; i < POWER(ghistoryBits); i++)
		  {
			gbht[i] = NOTTAKEN;
			choicer[i] = WSG;
		  }
	  }
    case CUSTOM:
	  {
		  ghr = 0;
		  percep_tab = (int**)malloc(sizeof(int*)*POWER(cus_pcIndexBits));

		  for(int i = 0; i < POWER(cus_pcIndexBits); i++)
		  {
			  percep_tab[i] = (int*)malloc(sizeof(int*)*(cus_ghistoryBits+1));
		  }

		  for(int i = 0; i < POWER(cus_pcIndexBits); i++)
		  {
			  for(int j = 0; j < (cus_ghistoryBits+1); j++)
			  {
				  percep_tab[i][j] = 0;
			  }
		  }
		
	  }
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t gshare_make_prediction(uint32_t pc)
{
  //
  // Gshare_make_prediction
  //

  uint32_t addr = ((pc^ghr) & mask(ghistoryBits));
  uint8_t predict = (gbht[addr] & 0x03);

  if(predict > WN)
  {
	return TAKEN;
  }
  else
  {
   return NOTTAKEN;
  }
}

uint8_t local_make_prediction(uint32_t pc)
{
  //
  // Local_make_prediction
  //
  bhr = (pc & mask(pcIndexBits));
  uint32_t addr = pht[bhr];
  addr = (addr & mask(lhistoryBits));
  uint8_t predict = (lbht[addr] & 0x03);

  if(predict > WN)
  {
	return TAKEN;
  }
  else
  {
   return NOTTAKEN;
  }
}

uint8_t global_make_prediction(uint32_t pc)
{
  //
  // Global_make_prediction
  //
  uint32_t addr = (ghr & mask(ghistoryBits));
  uint8_t predict = (gbht[addr] & 0x03);

  if(predict > WN)
  {
	return TAKEN;
  }
  else
  {
   return NOTTAKEN;
  }
}


uint8_t tournament_make_prediction(uint32_t pc)
{
  //
  // Tournament_make_prediction
  //
  uint32_t addr = (ghr & mask(ghistoryBits));
  uint8_t predict = (choicer[addr] & 0x03);

  if(predict > WSG)
  {
	return local_make_prediction(pc);
  }
  else
  {
	return global_make_prediction(pc);
  }
}

uint8_t perceptron_make_prediction(uint32_t pc)
{
	uint32_t addr = (pc & mask(cus_pcIndexBits));
	uint32_t cur_history = (ghr & mask(cus_ghistoryBits));
	int y = percep_tab[addr][cus_ghistoryBits];
	int w = 0;

	for(int j = 0; j < cus_ghistoryBits; j++)
	{
		w = percep_tab[addr][j];
		if(cur_history & 0x01)
		{
			y = y + w;
		}
		else
		{
			y = y - w;
		}
		cur_history = (cur_history >> 1);
	}

	if(y >= 0)
	{
		return TAKEN;
	}
	else
	{
		return NOTTAKEN;
	}
}

uint8_t make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
		return gshare_make_prediction(pc);
    case LOCAL:
		return local_make_prediction(pc);
    case TOURNAMENT:
		return tournament_make_prediction(pc);
    case CUSTOM:
		return perceptron_make_prediction(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
//
void gshare_train_predictor(uint32_t pc, uint8_t outcome)
{
	  //
	  //Gshare Predictor training
	  //
	  // unsigned short addr = ((pc^ghr) & mask(ghistoryBits));
	  // unsigned short predict = (gbht[addr] & 0x03);

	  uint32_t addr = ((pc^ghr) & mask(ghistoryBits));
	  uint8_t predict = (gbht[addr] & 0x03);

	  if(outcome)
	  {
		if(predict <= WT)
		{
		  predict++;
		}
	  }
	  else
	  {
		if(predict > SN)
		{
		  predict--;
		}
	  }

	  gbht[addr] = predict;

	  // shift GHR left
	  ghr = (ghr << 1);
	  ghr = (ghr | outcome);
}

void global_train_predictor(uint32_t pc, uint8_t outcome)
{
	  //
	  //Global Predictor training
	  //
	  uint32_t addr = (ghr & mask(ghistoryBits));
	  uint8_t predict = (gbht[addr] & 0x03);

	  if(outcome)
	  {
		if(predict <= WT)
		{
		  predict++;
		}
	  }
	  else
	  {
		if(predict > SN)
		{
		  predict--;
		}
	  }

	  gbht[addr] = predict;

	  // shift GHR left
	  ghr = (ghr << 1);
	  ghr = (ghr | outcome);
}

void local_train_predictor(uint32_t pc, uint8_t outcome)
{
	  //
	  //Local Predictor training
	  //
	  bhr = (pc & mask(pcIndexBits));
	  uint32_t addr = pht[bhr];
	  addr = (addr & mask(lhistoryBits));
	  uint8_t predict = (lbht[addr] & 0x03);

	  if(outcome)
	  {
		if(predict <= WT)
		{
		  predict++;
		}
	  }
	  else
	  {
		if(predict > SN)
		{
		  predict--;
		}
	  }

	  lbht[addr] = predict;

	  // shift entry in PHT left
	  pht[bhr] = (pht[bhr] << 1);
	  pht[bhr] = (pht[bhr] | outcome);
}

void tournament_train_predictor(uint32_t pc, uint8_t outcome)
{
	  //
	  //Tournament Predictor training
	  //
	  uint32_t addr = (ghr & mask(ghistoryBits));
	  uint8_t predict = (choicer[addr] & 0x03);

	  uint8_t global_predict = global_make_prediction(pc);
	  uint8_t local_predict = local_make_prediction(pc);

	  if(global_predict != outcome)
	  {
		  if(local_predict == outcome)
		  {
				if(predict <= WSL)
				{
				  predict++;
				}
		  }
	  }
	  else
	  {
		  if(local_predict != outcome)
		  {
				if(predict > SSG)
				{
				  predict--;
				}
		  }
	  }

	  choicer[addr] = predict;

	  // update global and local tables
	  global_train_predictor(pc, outcome);
	  local_train_predictor(pc, outcome);
}


void perceptron_train_predictor(uint32_t pc, uint8_t outcome)
{
	//
	// perceptron train
	//
	uint32_t addr = (pc & mask(cus_pcIndexBits));
	uint32_t cur_history = (ghr & mask(cus_ghistoryBits));

	int y = percep_tab[addr][cus_ghistoryBits];
	int w = 0;
	int t = ((outcome)? 1:-1);

	for(int j = 0; j < cus_ghistoryBits; j++)
	{
		w = percep_tab[addr][j];
		if(cur_history & 0x01)
		{
			y = y + w;
		}
		else
		{
			y = y - w;
		}
		cur_history = (cur_history >> 1);
	}

	int predict = ((y >= 0)? 1: -1);
	
	cur_history = (ghr & mask(cus_ghistoryBits));

	if((predict != t) || (check_threshold(y) > 0))
	{
		percep_tab[addr][cus_ghistoryBits] = set_limit(percep_tab[addr][cus_ghistoryBits] + t);

		for(int j = 0; j < cus_ghistoryBits; j++)
		{
			w = percep_tab[addr][j];

			if(cur_history & 0x01)
			{
				percep_tab[addr][j] = set_limit(w+t);
			}
			else
			{
				percep_tab[addr][j] = set_limit(w-t);
			}

			cur_history = (cur_history >> 1);
		}
	}

	ghr = (ghr << 1);
	ghr = (ghr | outcome);

}


void train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  // main part of train_predictor
  //
  switch (bpType) {
    case GSHARE:
		gshare_train_predictor(pc, outcome);
    case LOCAL:
		local_train_predictor(pc, outcome);
    case TOURNAMENT:
		tournament_train_predictor(pc, outcome);
    case CUSTOM:
		perceptron_train_predictor(pc, outcome);
    default:
      break;
  }
}
