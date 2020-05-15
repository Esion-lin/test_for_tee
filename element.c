#include "element.h"
uint32_t double_item(struct Data* data){

	uint8_t* new_label = (uint8_t*)malloc((data->item_capacity)*2*LABEL_LEN);
	printf("check point 3\n");
	uint8_t* new_pos = (uint8_t*)malloc(data->item_capacity*2*POS_LEN); 
	
	memcpy(new_label, data->label, (data->label_size)*LABEL_LEN);
	memcpy(new_pos, data->pos, (data->label_size)*POS_LEN);

	data->item_capacity = 2*data->item_capacity;
	free(data->label);
	free(data->pos);

	data->pos = new_pos;
	data->label = new_label;
	return 0;
}
uint32_t double_data(struct Data* data){
	uint8_t* new_data = (uint8_t*)malloc(data->data_capacity*2);
	memcpy(new_data, data->data, data->data_len);
	data->data_capacity = 2*data->data_capacity;
	free(data->data);
	data->data = new_data;
	return 0;
}
//query Data using label name, return index of label name.
uint32_t index_of(struct Data data, uint8_t* name, uint32_t name_len){
	uint8_t* label_tmp = data.label;
	uint32_t ans = data.label_size;
	for(int i = 0; i < data.label_size; i ++){
		if(memcmp(name, label_tmp + (i*LABEL_LEN), LABEL_LEN) == 0){
			ans = i; break;
		}
	}

	label_tmp = NULL;
	return ans;
	
}
//Use subscripts to access data, return stream of data
uint32_t get_data(struct Data data, uint32_t index, uint32_t* ele_size, uint8_t* ele){
	if(index >= data.label_size) return index;
	uint32_t pos;
	uint32_t data_size;
	memcpy(&pos, data.pos + (index * POS_LEN), POS_LEN);
	memcpy(&data_size, data.data + pos, sizeof(uint32_t));

	if(data_size > *ele_size){
		return data_size;
	}
	*ele_size = data_size;
	memcpy(ele, data.data + pos + sizeof(uint32_t), *ele_size);

	return 0;
}
uint32_t put_data(struct Data *data, uint32_t index, uint32_t ele_size, uint8_t* ele){
	if(index >= data->label_size) return index;
	uint32_t pos;
	uint32_t data_size;
	memcpy(&pos, data->pos + (index * POS_LEN), POS_LEN);
	memcpy(&data_size, data->data + pos, sizeof(uint32_t));
	if(ele_size > data_size){
		return data_size;
	}
	memcpy(data->data + pos, &ele_size, sizeof(uint32_t));
	memcpy(data->data + pos + sizeof(uint32_t), ele, ele_size);
	return 0;

}
/*TODO: Add boundary check*/
uint32_t add_data(struct Data* data, uint8_t *name, uint32_t name_len, uint8_t* data_in, uint32_t data_len){
	uint32_t idx = index_of(*data, name, name_len);
	if(idx == data->label_size){
		if(data->label_size + 1 > data->item_capacity){
			printf("expend data label\n");
			double_item(data);
		}
		printf("check point 0\n");
		memcpy(data->label + data->label_size * LABEL_LEN, name, name_len);
		memcpy(data->pos + data->label_size * POS_LEN, &data->data_len, POS_LEN);
		data->label_size += 1;
		printf("check point 1\n");
		if(data->data_len + sizeof(uint32_t) + data_len > data->data_capacity){
			printf("expend data\n");
			double_data(data);
		}
		memcpy(data->data + data->data_len, &data_len, sizeof(uint32_t));
		memcpy(data->data + data->data_len + sizeof(uint32_t), data_in, data_len);
		data->data_len += (data_len + sizeof(uint32_t));
	}else{
		return put_data(data, idx, data_len, data_in);
	}
	return 0;
}
/*TODO: Add boundary check*/
uint32_t deserialize(struct Data data, uint8_t* label, uint32_t* label_len, uint8_t* data_out, uint32_t* data_len){
	memcpy(label, data.label, data.label_size*LABEL_LEN);
	memcpy(data_out, data.pos, data.label_size*POS_LEN);
	memcpy(data_out + data.label_size*POS_LEN, data.data, data.data_len);
	*label_len = data.label_size;
	*data_len = data.label_size*POS_LEN + data.data_len;
	return 0;
}
/*TODO: Add boundary check*/
uint32_t serialize(struct Data* data, uint8_t* label, uint32_t label_len, uint8_t* data_in, uint32_t data_len){
	data->label_size = label_len;
	memcpy(data->label, label, label_len*LABEL_LEN);
	memcpy(data->pos, data_in, label_len*POS_LEN);
	data->data_len = data_len - label_len*POS_LEN;
	memcpy(data->data, data_in + label_len*POS_LEN, data->data_len);
	return 0;
}
/*decode protocol*/
uint32_t run_code(struct Data* data, struct Code* code, struct Tuple* last_tp, uint8_t* label){
	/*optimise store*/
	/*query nower idex of S*/
	uint32_t now_pos;
	memcpy(&now_pos, code->pos_s + (POS_LEN*code->now_pos), POS_LEN);
	code->now_pos ++;
	//printf("now_pos is %u, S[%d]type is %u\n",code->now_pos,now_pos,code->S[now_pos]);
	if(code->S[now_pos] < 99){
		/*recover data of label1\label2\label3*/
		uint8_t label1[LABEL_LEN];
		uint32_t type1;
		uint8_t label2[LABEL_LEN];
		uint32_t type2;
		uint8_t label3[LABEL_LEN];
		uint32_t type3;
		uint8_t data1[sizeof(uint32_t)];
		uint32_t data1_len = sizeof(uint32_t);
		uint8_t data2[sizeof(uint32_t)];
		uint32_t data2_len = sizeof(uint32_t);
		
		memcpy(label1, code->S + now_pos + 1, LABEL_LEN);
		type1 = code->S[now_pos + 1 + LABEL_LEN];
		memcpy(label2, code->S + now_pos + 2 + LABEL_LEN, LABEL_LEN);
		type2 = code->S[now_pos + 2 + 2*LABEL_LEN];
		memcpy(label3, code->S + now_pos + 3 + 2 * LABEL_LEN, LABEL_LEN);
		type3 = code->S[now_pos + 3 + 3*LABEL_LEN];
		if(0 != get_data(*data, index_of(*data, label1, LABEL_LEN), &data1_len, data1)){
			printf("error[label1]: cannot get right data in Binary/Arithmatic/Logic cmd.\n");
			return 100;
		}
		if(0 != get_data(*data, index_of(*data, label2, LABEL_LEN), &data2_len, data2)){
			printf("error[label2]: cannot get right data in Binary/Arithmatic/Logic cmd.\n");
			return 100;
		}
		// if(type3 < (type2>type1?type2:type1)){
		// 	printf("error[structure]: cannot convert bigger size data to smaller size data.\n");
		// 	return 101;
		// }
		if(code->S[now_pos] < 10){
			//binary
			uint8_t data3[sizeof(uint32_t)];
			uint32_t data3_len = sizeof(uint32_t);
			uint32_t tar,src1,src2;
			convert(src1,data1);
			convert(src2,data2);
			switch(code->S[now_pos]){
				case AND_OP:{
					tar = src1 & src2;
				}
				break;
				case NOT_OP:{
					tar = ~src1;
				}
				break;
				case OR_OP:{
					tar = src1 | src2;
				}
				break;
				case NAND_OP:{
					tar = src1 ^ src2;
				}
				break;
				case SHF_LE:{
					tar = src1 << src2;
				}
				break;
				case SHF_RI:{
					tar = src1 >> src2;
				}
				break;

			}
			memcpy(data3, &tar, sizeof(uint32_t));
			add_data(data, label3, LABEL_LEN, data3, sizeof(uint32_t));

		}else if(code->S[now_pos] < 20){
			printf("do arithmatic\n");
			//Arithmaic
			uint8_t data3[sizeof(uint32_t)];
			uint32_t data3_len = sizeof(uint32_t);
			float tar,src1,src2;
			convert(src1,data1);
			convert(src2,data2);
			switch(code->S[now_pos]){
				
				case ADD_OP:{
					tar = src1 + src2;
				}
				break;
				case SUB_OP:{
					tar = src1 - src2;
				}
				break;
				case MUL_OP:{
					tar = src1 * src2;
				}
				break;
				case DIV_OP:{
					tar = src1 / src2;
				}
				break;
				case POW_OP:{
					tar = pow(src1, src2);
				}
				break;
			}
			printf("get middle data%f %f %f\n",src1, src2, tar);
			memcpy(data3, &tar, sizeof(float));
			add_data(data, label3, LABEL_LEN, data3, sizeof(float));
		}else{
			//Logic
			uint8_t data3[1];
			uint32_t data3_len = 1;
			float src1,src2;bool tar;
			convert(src1,data1);
			convert(src2,data2);
			switch(code->S[now_pos]){
				case GREAT_OP:{
					tar = src1 > src2;
				}
				break;
				case GE_OP:{
					tar = src1 >= src2;
				}
				break;
				case LESS_OP:{
					tar = src1 < src2;
				}
				break;
				case LE_OP:{
					tar = src1 <= src2;
				}
				break;
				case EQ_OP:{
					tar = src1 == src2;
				}
				break;
			}
			data3[0] = tar;
			add_data(data, label3, LABEL_LEN, data3, 1);
		}
		
	}
	else if(code->S[now_pos] == 99){
		printf("do goto\n");
		uint8_t label[LABEL_LEN];
		uint8_t label_data[1];
		uint32_t label_data_len = 1;
		uint32_t tar_pos;
		memcpy(label, code->S + now_pos + 1, LABEL_LEN);
		if(0 != get_data(*data, index_of(*data, label, LABEL_LEN), &label_data_len, label_data)){
			printf("error[label]: cannot get right data in Binary/Arithmatic/Logic cmd.\n");
			return 100;
		}
		if(label_data[0] == 1){
			memcpy(&tar_pos, code->S + now_pos + 1 + LABEL_LEN, sizeof(uint32_t));
			code->now_pos = tar_pos;
		}
		
	}
	else{
		switch(code->S[now_pos]){
			case COV:{
				printf("do cov\n");
				/*recover the structure*/
				uint32_t step = now_pos + 1;
				uint8_t label[LABEL_LEN];
				struct Shape data_shape;
				uint8_t out_label[LABEL_LEN];
				uint32_t kernel_size;
				bool is_pending;
				uint32_t stride;
				struct Shape kernel_shape;
				uint8_t ex_for_tee;
				uint32_t W_pos;
				memcpy(label, code->S + step, LABEL_LEN);
				step += LABEL_LEN;
				data_shape.l = code->S[step]*256 + code->S[step+1];
				step += 2;
				data_shape.w = code->S[step]*256 + code->S[step+1];
				step += 2;
				data_shape.h = code->S[step]*256 + code->S[step+1];
				step += 2;
				data_shape.size = data_shape.l * data_shape.w * data_shape.h;
				printf("[%u, %u, %u]",data_shape.l , data_shape.w , data_shape.h);
				memcpy(out_label, code->S + step, LABEL_LEN);
				step += LABEL_LEN;
				kernel_size = code->S[step++]*256 + code->S[step++]; 
				is_pending = (code->S[step++] == 1);
				stride = code->S[step++];
				kernel_shape.l = code->S[step]*256 + code->S[step+1];
				step += 2;
				kernel_shape.w = code->S[step]*256 + code->S[step+1];
				step += 2;
				kernel_shape.h = code->S[step]*256 + code->S[step+1];
				step += 2;
				kernel_shape.size = kernel_shape.l * kernel_shape.w * kernel_shape.h;
				printf("[%u, %u, %u]",kernel_shape.l , kernel_shape.w , kernel_shape.h);
				ex_for_tee = code->S[step++];
				memcpy(&W_pos, code->S + step, POS_LEN);
				step += POS_LEN;
				/*recover the data*/
				struct Tuple image;
				image.shape = data_shape;
				image.data = (float*)malloc(data_shape.size * sizeof(float));

				uint32_t check_len = data_shape.size * sizeof(float);
				uint8_t* check_data = (uint8_t*)malloc(check_len);
				if(0!=get_data(*data, index_of(*data, label, LABEL_LEN), &check_len, check_data)){
					printf("error[label]: cannot get right data in cov cmd.\n");
					free(check_data);
					free(image.data);
					return 100;
				}
				if(check_len != data_shape.size * sizeof(float)){
					printf("data error.shape not match\n");
					free(check_data);
					free(image.data);
					return 200;
				}
				memcpy(image.data,check_data,check_len);
				free(check_data);
				/*recover the weight*/
				struct Tuple* kernels = (struct Tuple*)malloc(sizeof(struct Tuple)*kernel_size);
				for(int i = 0; i < kernel_size; i++){
					kernels[i].shape = kernel_shape;
					kernels[i].data = (float*)malloc(kernel_shape.size*sizeof(float));
					memcpy(kernels[i].data, code->W + i*kernel_shape.size*sizeof(float), kernel_shape.size*sizeof(float));
				}
				printf("start cov\n");
				struct Tuple ans = convolution_embed(image, kernels, kernel_size, is_pending, stride);

				printf("ans's shape[%u,%u,%u]\n",ans.shape.w,ans.shape.l,ans.shape.h);
				for(int i = 0; i <113;i++){
					printf("%f ",ans.data[i]);
				}
				printf("\n");
				/*free image and kernels*/
				free(image.data);image.data = NULL;
				for(int i = 0; i < kernel_size; i++){
					free(kernels[i].data); 
				}
				free(kernels);
				kernels = NULL;

			}
			break;
			case POOLING:{
				printf("do pooling\n");
			}
			break;
			case RELU:{

			}
			break;
			case BN_ID:{

			}
			break;
			case FC_ID:{

			}
			break;
			case SHORTCUT:{

			}
			break;

		}
	}
	
}
int main(){
	/*
	+ l1 l2 l3
	- l3 l2 l4
	== l4 l1 l5
	cov org 225 225 3 lay1 64 true 2 3 3 3 false 0
	*/
	struct Code code;

	code.code_size = 4;
	code.pos_s = (uint8_t*)malloc(code.code_size*POS_LEN);
	uint8_t* Structure = (uint8_t*)malloc(INS_LEN*3 + COV_LEN);
	
	code.now_pos = 0;
	uint32_t itr = 0;
	


	char label1[LABEL_LEN] = "l1";
	float l1 = 18;
	float l2 = 22;
	char label2[LABEL_LEN] = "l2";
	char label3[LABEL_LEN] = "l3";
	char label4[LABEL_LEN] = "l4";
	char label5[LABEL_LEN] = "l5";
	memcpy(code.pos_s,&itr,POS_LEN);
	Structure[itr++] = ADD_OP;
	memcpy(Structure + itr, label1, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;
	memcpy(Structure + itr, label2, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;
	memcpy(Structure + itr, label3, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;

	memcpy(code.pos_s + POS_LEN,&itr,POS_LEN);
	Structure[itr++] = SUB_OP;
	memcpy(Structure + itr, label3, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;
	memcpy(Structure + itr, label2, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;
	memcpy(Structure + itr, label4, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;

	memcpy(code.pos_s + POS_LEN*2,&itr,POS_LEN);
	Structure[itr++] = EQ_OP;
	memcpy(Structure + itr, label4, LABEL_LEN);
	itr += LABEL_LEN;

	Structure[itr++] = FLOAT_TP;
	memcpy(Structure + itr, label1, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = FLOAT_TP;
	memcpy(Structure + itr, label5, LABEL_LEN);
	itr += LABEL_LEN;

	Structure[itr++] = FLOAT_TP;

	printf("next add structure of cov\n");
	memcpy(code.pos_s + POS_LEN*3,&itr,POS_LEN);
	Structure[itr++] = COV;
	char cov_label[LABEL_LEN] = "org";
	memcpy(Structure + itr, cov_label, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = 225/256;
	Structure[itr++] = 225%256;
	Structure[itr++] = 225/256;
	Structure[itr++] = 225%256;
	Structure[itr++] = 3/256;
	Structure[itr++] = 3%256;
	char cov_label_out[LABEL_LEN] = "lay1";
	memcpy(Structure + itr, cov_label_out, LABEL_LEN);
	itr += LABEL_LEN;
	Structure[itr++] = 64/256;
	Structure[itr++] = 64%256;
	Structure[itr++] = 1;
	Structure[itr++] = 2;
	Structure[itr++] = 5/256;
	Structure[itr++] = 5%256;
	Structure[itr++] = 5/256;
	Structure[itr++] = 5%256;
	Structure[itr++] = 3/256;
	Structure[itr++] = 3%256;
	Structure[itr++] = 1;
	uint32_t wpos = 0;
	memcpy(Structure + itr, &wpos, POS_LEN);
	itr += POS_LEN;

	code.W_len = 5*5*3*64*sizeof(float);
	code.W = (uint8_t*)malloc(code.W_len);
	float arr[75];
	for(int i = 0; i < 75; i++){
		arr[i] = 1;
	}
	for(int i = 0; i < 64; i++){
		memcpy(code.W + i * 5*5*3*sizeof(float), arr, 5*5*3*sizeof(float));
	}	
	printf("itr is %u\n",itr);
	code.S = Structure;
	code.S_len =  19*3;

	struct Data data;
	data.label_size = 0;
	data.item_capacity = 6;
	data.data_len = 0;
	data.data_capacity = 8*5 + 225*225*3*sizeof(float);
	data.label = (uint8_t*)malloc(data.item_capacity*LABEL_LEN);
	data.pos = (uint8_t*)malloc(data.item_capacity*POS_LEN);
	data.data = (uint8_t*)malloc(data.data_capacity);
	uint8_t ll1[4],ll2[4];
	uint32_t l1_len = 4,l2_len = 4;
	memcpy(ll1, &l1, 4);
	memcpy(ll2, &l2, 4);
	add_data(&data, label1, LABEL_LEN, ll1, l1_len);
	add_data(&data, label2, LABEL_LEN, ll2, l2_len);
	float* image_data = (float*)malloc(225*225*3*sizeof(float));
	for(int i = 0; i < 225*225*3; i++){
		image_data[i] = i/225/225;
	}

	uint32_t image_len = 225*225*3*sizeof(float);
	add_data(&data, cov_label, LABEL_LEN, (uint8_t*)image_data, image_len);
	run_code(&data, &code);
	run_code(&data, &code);
	run_code(&data, &code);
	uint8_t ans[1];
	uint32_t ans_len = 1;
	if(0 == get_data(data, index_of(data, label5, LABEL_LEN), &ans_len, ans)){
		printf("result is %u\n", ans[0]);
	}
	run_code(&data, &code);
	printf("free\n");
	free(image_data);
	free(code.pos_s);
	free(code.W);
	free(Structure);
	free(data.label);
	free(data.pos);
	free(data.data);
	return 0;
}