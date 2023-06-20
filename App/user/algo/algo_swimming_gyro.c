
#include <stdio.h>
#include <string.h>
#include <math.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

#define STROKE_TYPE_BUTTERFLY								0
#define STROKE_TYPE_BACKSTROKE								1
#define STROKE_TYPE_BREASTSTROKE							2
#define STROKE_TYPE_FREESTYLE								3

#define CREST_COUNT_THRESHOLD								50

long long gyro_x_value_pos=0;
long long gyro_x_value_neg=0; 

long long gyro_z_value_pos=0;
long long gyro_z_value_neg=0; 

struct crest_trough_st{
	int is_crest;
	int start;
	int end;
}crest_breaststroke[150], crest_freestyle[150];

//data struct for breaststroke
struct brs_control_st{
	int state;
	int point_count;
	int not_first_point ;
	int period_mid; 				//the middle value of all periods
	int turn_back_cnt;
	int last_turn_back;
	int last_period_end;
    int peak_unexpected;			//for stroke type recognition
}brs_ctrl;

//data struct for freestyle
struct frs_control_st{
	int point_count;
	int not_first_point ;
	int period_mid; 				//the middle value of all periods
	int turn_back_cnt;
	int last_turn_back;
	int last_period_end;
}frs_ctrl;

static int brs_crest_count=0, frs_crest_count = 0;
static int period_breaststroke[150], period_freestyle[150];
static int brs_period_cnt=0, frs_period_cnt = 0;
static int stroke_type = -1;


//Input: gyro x or z axis data
void breaststroke_recognition(int gyro_x, int gyro_z)
{
	if(gyro_x >= 0 ){
		gyro_x_value_pos += gyro_x;
	}
	else
	{
		gyro_x_value_neg += gyro_x;
	}
	if (gyro_z >= 0){
		gyro_z_value_pos += gyro_z;
	}
	else{
		gyro_z_value_neg += gyro_z; 
	}
}

void stroke_type_res(double ratio)
{
	printf("ratio: %lf\n",ratio);
}


void swim_finished()
{
	switch(stroke_type){
	case STROKE_TYPE_BUTTERFLY:
		
		break;
	case STROKE_TYPE_BACKSTROKE:
		break;
	case STROKE_TYPE_BREASTSTROKE:
		printf("period: %d, turn-back: %d, unexpected peak: %d\n", brs_period_cnt, brs_ctrl.turn_back_cnt, brs_ctrl.peak_unexpected);
		break;
	case STROKE_TYPE_FREESTYLE:
		printf("period: %d, turn-back: %d\n", frs_period_cnt, frs_ctrl.turn_back_cnt);
		break;
	}
}

//Input: gyro Y axis data
void breaststroke_analyze(int gyro_x, int gyro_y, int gyro_z)
{
	int i;
    
	int crest_detected=0;
	double ratio;
	
	if(stroke_type >= 0 && stroke_type != STROKE_TYPE_BREASTSTROKE)
	{
		return;
	}
	
	//Jurge the stroke type.
	breaststroke_recognition(gyro_x,gyro_z);

	//crest
	if(gyro_y > 25000){
		if (brs_ctrl.not_first_point == 0 ){
			crest_detected = 1;
			crest_breaststroke[brs_crest_count].is_crest = 1;
			crest_breaststroke[brs_crest_count].start = brs_ctrl.point_count;
			crest_breaststroke[brs_crest_count].end = brs_ctrl.point_count;
			brs_ctrl.not_first_point = 1;
			crest_detected = 1;
		}
		else if( crest_breaststroke[brs_crest_count].is_crest == 1 && crest_breaststroke[brs_crest_count].end >= brs_ctrl.point_count - 4){
			crest_breaststroke[brs_crest_count].end = brs_ctrl.point_count;
		}
		else{
			if(brs_crest_count == CREST_COUNT_THRESHOLD){
				ratio = gyro_x_value_neg/(double)(-gyro_x_value_pos) > gyro_z_value_neg/(double)(-gyro_z_value_pos)?
							gyro_x_value_neg/(double)(-gyro_x_value_pos) : gyro_z_value_neg/(double)(-gyro_z_value_pos);
				stroke_type_res(ratio);
				if(ratio > 1.8){
					stroke_type = STROKE_TYPE_BREASTSTROKE;
				}
					
				if(brs_period_cnt){
					#if 0
					vector<int> period_vec (period_breaststroke, period_breaststroke+brs_period_cnt);
					sort(period_vec.begin(), period_vec.end(), cmp);
					brs_ctrl.period_mid = period_vec[brs_period_cnt/2];
					#endif
					brs_ctrl.period_mid = period_breaststroke[10];
					printf("mid: %d\n", brs_ctrl.period_mid);
					for(i=0;i<brs_period_cnt;i++)
					{
						if(  period_breaststroke[i] > brs_ctrl.period_mid * 1.2 &&
							i > 10 &&  i - brs_ctrl.last_turn_back > 10) {
							printf("turn-back: %d, period: %d\n", brs_period_cnt, period_breaststroke[brs_period_cnt]);
							brs_ctrl.turn_back_cnt++;
							brs_ctrl.last_turn_back = i;
						}
					}
				}
			}
			
			if(brs_crest_count <= CREST_COUNT_THRESHOLD){
				brs_crest_count++;
			}
			crest_detected = 1;
			crest_breaststroke[brs_crest_count].is_crest = 1;
			crest_breaststroke[brs_crest_count].start = brs_ctrl.point_count;
			crest_breaststroke[brs_crest_count].end = brs_ctrl.point_count;
		}
	}
	//through
	else if (gyro_y < -25000){
		if (brs_ctrl.not_first_point == 0 ){
			crest_detected = 1;
			crest_breaststroke[brs_crest_count].is_crest = 0;
			crest_breaststroke[brs_crest_count].start = brs_ctrl.point_count;
			crest_breaststroke[brs_crest_count].end = brs_ctrl.point_count;
			brs_ctrl.not_first_point = 1;
			crest_detected = 1;
			brs_crest_count++;
		}
		else if(brs_crest_count != 0 && crest_breaststroke[brs_crest_count].is_crest == 0 && crest_breaststroke[brs_crest_count].end >= brs_ctrl.point_count - 4){
			crest_breaststroke[brs_crest_count].end = brs_ctrl.point_count;
		}
		else{
			if(brs_crest_count == CREST_COUNT_THRESHOLD){
				ratio = gyro_x_value_neg/(double)(-gyro_x_value_pos) > gyro_z_value_neg/(double)(-gyro_z_value_pos)?
							gyro_x_value_neg/(double)(-gyro_x_value_pos) : gyro_z_value_neg/(double)(-gyro_z_value_pos);
				stroke_type_res(ratio);
				if(ratio > 1.8){
					stroke_type = STROKE_TYPE_BREASTSTROKE;
				}
				
				if(brs_period_cnt){
					#if 0
					vector<int> period_vec (period_breaststroke, period_breaststroke+brs_period_cnt);
					sort(period_vec.begin(), period_vec.end(), cmp);
					brs_ctrl.period_mid = period_vec[brs_period_cnt/2];
					#endif
					brs_ctrl.period_mid = period_breaststroke[10];
					printf("mid: %d\n", brs_ctrl.period_mid);
					for(i=0;i<brs_period_cnt;i++)
					{
						if(  period_breaststroke[brs_period_cnt] > brs_ctrl.period_mid * 1.2 &&
							i > 10 &&  i - brs_ctrl.last_turn_back > 10) {
							printf("turn-back: %d, period: %d\n", brs_period_cnt, period_breaststroke[brs_period_cnt]);
							brs_ctrl.turn_back_cnt++;
							brs_ctrl.last_turn_back = i;
						}
					}
				}
			}
			
			if(brs_crest_count <= CREST_COUNT_THRESHOLD){
				brs_crest_count++;
			}
			crest_detected = 1;
			crest_breaststroke[brs_crest_count].is_crest = 0;
			crest_breaststroke[brs_crest_count].start = brs_ctrl.point_count;
			crest_breaststroke[brs_crest_count].end = brs_ctrl.point_count;
		}
	}
	
	brs_ctrl.point_count++;

//	printf("crest count: %d\n", crest_count);
//	for(i=0;i<crest_count;i++){
//		printf("peak %d: is crest:%d start %d, end %d.", i, crest_breaststroke[i].is_crest, crest_breaststroke[i].start,crest_breaststroke[i].end);
//	}
//	printf("\n");
	
	
	//calcaluate stroke
	/*
	Assume that there is a crest follow a trough in one period of breaststroke, sometimes there is a trough at last.
	The sequence is trough-crest-(trough), to detect this pattern I use state machine for convinience.
	*/
	if(crest_detected)
	{
		switch(brs_ctrl.state){
		//expect a through
		case 0:
			if(crest_breaststroke[brs_crest_count].is_crest == 0){
				brs_ctrl.state = 2;
			}
			else{
				brs_ctrl.peak_unexpected++;
				printf("peack unexpected, point: %d \n",brs_ctrl.point_count);
			}
		    break;
		//expect a crest or a through
		case 1:
			if(crest_breaststroke[brs_crest_count].is_crest == 0){
				brs_ctrl.state = 2;
			}
			else{
				if(brs_ctrl.last_period_end != -1){
					if(brs_crest_count <= CREST_COUNT_THRESHOLD){
						period_breaststroke[brs_period_cnt] = crest_breaststroke[brs_crest_count].start - brs_ctrl.last_period_end;
						printf("period %d: %d, ", brs_period_cnt, period_breaststroke[brs_period_cnt]);
					}
					else{
						period_breaststroke[CREST_COUNT_THRESHOLD+1] = crest_breaststroke[brs_crest_count].start - brs_ctrl.last_period_end;
						printf("period %d: %d, ", brs_period_cnt, period_breaststroke[CREST_COUNT_THRESHOLD+1]);
					}
					if(brs_crest_count > CREST_COUNT_THRESHOLD && period_breaststroke[CREST_COUNT_THRESHOLD+1] > brs_ctrl.period_mid*1.2
					&& brs_period_cnt - brs_ctrl.last_turn_back > 10){
						brs_ctrl.last_turn_back = brs_period_cnt;
						brs_ctrl.turn_back_cnt++;
						printf("turn-back: %d, period: %d\n", brs_period_cnt, period_breaststroke[brs_period_cnt]);
					}
				}
				brs_ctrl.last_period_end = crest_breaststroke[brs_crest_count].start;
				brs_period_cnt++;
				brs_ctrl.state = 3;
			}
			break;
		//expect a crest
		case 2:
			if(crest_breaststroke[brs_crest_count].is_crest == 1){
				if(brs_ctrl.last_period_end != -1){
					if(brs_crest_count <= CREST_COUNT_THRESHOLD){
						period_breaststroke[brs_period_cnt] = crest_breaststroke[brs_crest_count].start - brs_ctrl.last_period_end;
						printf("period %d: %d, ", brs_period_cnt, period_breaststroke[brs_period_cnt]);
					}
					else{
						period_breaststroke[CREST_COUNT_THRESHOLD+1] = crest_breaststroke[brs_crest_count].start - brs_ctrl.last_period_end;
						printf("period %d: %d, ", brs_period_cnt, period_breaststroke[CREST_COUNT_THRESHOLD+1]);
					}
					if(brs_crest_count > CREST_COUNT_THRESHOLD && period_breaststroke[CREST_COUNT_THRESHOLD+1] > brs_ctrl.period_mid*1.2
					&& brs_period_cnt - brs_ctrl.last_turn_back > 10){
						brs_ctrl.last_turn_back = brs_period_cnt;
						brs_ctrl.turn_back_cnt++;
						printf("turn-back: %d, period: %d\n", brs_period_cnt, period_breaststroke[brs_period_cnt]);
					}
				}
				brs_ctrl.last_period_end = crest_breaststroke[brs_crest_count].start;
				brs_period_cnt++;
				brs_ctrl.state = 3;
			}
			else{
				brs_ctrl.peak_unexpected++;
				printf("peack unexpected, point: %d \n",brs_ctrl.point_count);
			}
			break;
		//expect a through
		case 3:
			if(crest_breaststroke[brs_crest_count].is_crest == 0){
				brs_ctrl.state = 1;
			}
			else{
				brs_ctrl.peak_unexpected++;
				printf("peack unexpected, point: %d \n",brs_ctrl.point_count);
				brs_ctrl.state = 0;
			}
			break;
		}
	}
	return ;
}

void freestyle_recognition()
{
	//Try the area of the gyro z, or left to the last when the other three stroke types are not satisfied. The measuring range should be expanded.

	
}

void freestyle_analyze(int gyro_x, int gyro_y, int gyro_z)
{
	int i;
	int crest_detected = 0; 
	
	if(stroke_type != -1 && stroke_type != STROKE_TYPE_FREESTYLE){
		return ;
	}
	
	freestyle_recognition();
	
	if (gyro_x < -28000){
		if (frs_ctrl.not_first_point == 0 ){
			crest_freestyle[frs_crest_count].is_crest = 0;
			crest_freestyle[frs_crest_count].start = frs_ctrl.point_count;
			crest_freestyle[frs_crest_count].end = frs_ctrl.point_count;
			frs_ctrl.not_first_point = 1;
			crest_detected = 1;
		}
		else if(frs_crest_count != 0 && crest_freestyle[frs_crest_count].is_crest == 0 && crest_freestyle[frs_crest_count].end >= frs_ctrl.point_count - 4){
			crest_freestyle[frs_crest_count].end = frs_ctrl.point_count;
		}
		else{
			if(frs_crest_count == CREST_COUNT_THRESHOLD){
				#if 0
				vector<int> period_vec (period_freestyle, period_freestyle+frs_period_cnt);
				sort(period_vec.begin(), period_vec.end(), cmp);
				frs_ctrl.period_mid = period_vec[frs_period_cnt/2];
				#endif
				brs_ctrl.period_mid = period_breaststroke[10];
				for(i=0;i<frs_period_cnt;i++)
				{
					if(  period_freestyle[i] > frs_ctrl.period_mid * 1.2 &&
						i > 10 &&  i - frs_ctrl.last_turn_back > 10) {
						frs_ctrl.turn_back_cnt++;
						frs_ctrl.last_turn_back = i;
					}
				}
				printf("turn-back: %d\n", frs_ctrl.turn_back_cnt);
			}
			if(frs_crest_count <= CREST_COUNT_THRESHOLD){
				frs_crest_count++;
			}
			crest_detected = 1;
			crest_freestyle[frs_crest_count].is_crest = 0;
			crest_freestyle[frs_crest_count].start = frs_ctrl.point_count;
			crest_freestyle[frs_crest_count].end = frs_ctrl.point_count;
		}
	}
	frs_ctrl.point_count++;
		
	if(crest_detected){ 
		if(frs_crest_count != 0){
			if(frs_period_cnt <= CREST_COUNT_THRESHOLD){
				period_freestyle[frs_period_cnt] = crest_freestyle[frs_crest_count].end - frs_ctrl.last_period_end;
				printf("period %d: %d,", frs_period_cnt, period_freestyle[frs_period_cnt]);
			}
			else{
				period_freestyle[CREST_COUNT_THRESHOLD+1] = crest_freestyle[frs_crest_count].end - frs_ctrl.last_period_end;
				printf("period %d: %d,", frs_period_cnt, period_freestyle[CREST_COUNT_THRESHOLD+1]);
			}
			frs_period_cnt++;
		}
		frs_ctrl.last_period_end = crest_freestyle[frs_crest_count].end;
		printf("peak %d: start %d, end %d.", frs_crest_count, crest_freestyle[frs_crest_count].start,crest_freestyle[frs_crest_count].end);
		
		if(frs_crest_count > CREST_COUNT_THRESHOLD){
			if(  period_freestyle[CREST_COUNT_THRESHOLD+1] > frs_ctrl.period_mid * 1.2 &&
				frs_period_cnt > 10 &&  frs_period_cnt - frs_ctrl.last_turn_back > 10) {
				frs_ctrl.turn_back_cnt++;
				frs_ctrl.last_turn_back = frs_period_cnt;
			}
		} 
	}

}

void butterfly_analyze()
{
}

void backstroke_analyze()
{
}

#if 0
int swimming_test() {
	FILE * f;
	int gyro_x, gyro_y, gyro_z;
	memset(&brs_ctrl, 0, sizeof(brs_ctrl));
	memset(&frs_ctrl, 0, sizeof(frs_ctrl));
	brs_ctrl.last_period_end = -1;
	frs_ctrl.last_period_end = -1;
	gyro_x_value_pos = 0;
	gyro_x_value_neg = 0;
	gyro_z_value_pos = 0;
	gyro_z_value_neg = 0;
    brs_crest_count=0, frs_crest_count = 0;
    brs_period_cnt=0, frs_period_cnt = 0;
    
	/*
	Assign different priorities to all axes to determine the stroke type.
	*/
	
	f = fopen("043_freestyle.txt","r");
	
//	while(fscanf(f, "%d%d%d",&gyro_x,&gyro_y, &gyro_z) != EOF){
//		breaststroke_analyze(gyro_x, gyro_y, gyro_z);
//	}
	
	while(fscanf(f, "%d%d%d",&gyro_x,&gyro_y, &gyro_z) != EOF){
		freestyle_analyze(gyro_x, gyro_y, gyro_z);
	}
	printf("period: %d, turn-back: %d\n", frs_period_cnt, frs_ctrl.turn_back_cnt);
	
//	butterfly_analyze();
	
//	backstroke_analyze();

	swim_finished();
	return 0;
}
#endif
