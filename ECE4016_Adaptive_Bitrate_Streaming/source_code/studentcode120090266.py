#Written by Nathan A-M =^)
#Buffer-based implementation using 
#A Buffer-based approach as a reference 

bitrate = 0 #used to save previous bitrate
runningFastStart = True
def student_entrypoint(Measured_Bandwidth, Previous_Throughput, Buffer_Occupancy, Available_Bitrates, Video_Time, Chunk, Rebuffering_Time, Preferred_Bitrate, Chunk_Item, Flag):
    #student can do whatever they want from here going forward
    global bitrate
    R_i = list(Available_Bitrates.items())
    R_i.sort(key=lambda tup: tup[1] , reverse=True)
    bitrate, B_delay = bufferbased(rate_prev=bitrate, buf_now= Buffer_Occupancy, r=Chunk['time']+1,R_i= R_i, chunk_item = Chunk_Item, fg = Flag, cur_time= Video_Time ) 
    return bitrate, B_delay

#helper function, to find the corresponding size of previous bitrate
def match(value, list_of_list): 
    for e in list_of_list:
        if value == e[1]:
            return e
            
#helper function, to find the previous bitrate
#if there's was no previous assume that it was the lowest possible value
def prevmatch(value, list_of_list): 
    for e in list_of_list:
        if value == e:
            return e
    return list_of_list[0]
        
        
deltaT = 50
deltaB = 1
B_min = 10
B_low = 20
B_high = 40
B_opt = (B_high + B_low) / 2
alpha1, alpah2, alpha3, alpha4, alpha5 = 0.75, 0.33, 0.5, 0.75, 0.9   
phi = 50   

def get_bitrate_interval(cur_time, chunk_item):
    if chunk_item == []:
        return 0
    pre_time = cur_time - deltaT
    totalT = 0
    totalRho = 0
    for item in chunk_item:
        # print(item)
        if item['end_time'] <= cur_time:
            thp = item['avg_throughput'] * phi / (item['end_time'] - item['begin_time'])
            time = min(cur_time, item['end_time']) - max(pre_time, item['begin_time'])
            if time > 1e-7:
                totalRho += thp * time
                totalT += time
    # print(totalRho, totalT)
    if totalT > 1e-7:
        return totalRho / totalT 
    else:
        return 0
            
    
def bufferbased(rate_prev, buf_now, r, R_i , chunk_item, fg, cur_time):
    '''
    Input: 
    rate_prev: The previously used video rate
    Buf_now: The current buffer occupancy 
    chunk_item: store information of chunk
    R_i: Array of bitrates of videos, key will be bitrate, and value will be the byte size of the chunk
    fg: whether buffer is increasing
    Output: 
    Rate_next: The next video rate, B_delay
    '''
    R_max = max(int(i[0]) for i in R_i)
    R_min = min(int(i[0]) for i in R_i)
    # rate_prev_list = list(int(i[0]) for i in R_i)
    # rate_prev = prevmatch(rate_prev_list,R_i)
    # print(buf_now)
    #set rate_plus to lowest reasonable rate
    if rate_prev == R_max:
        rate_plus = R_max
    else:
        more_rate_prev = list(int(i[0]) for i in R_i if int(i[0]) > rate_prev)
        if more_rate_prev == []:
            rate_plus = rate_prev[0]
        else: 
            rate_plus = min(more_rate_prev)
    
    #set rate_min to highest reasonable rate
    if rate_prev == R_min:
        rate_mins = R_min
    else:
        less_rate_prev= list(int(i[0]) for i in R_i if int(i[0]) < rate_prev)
        if less_rate_prev == []:
            rate_mins = rate_prev
        else: 
            rate_mins = max(less_rate_prev)
    
    #Buffer based Algorithm

    global runningFastStart
    B_delay = 0
    rho_t = get_bitrate_interval(cur_time, chunk_item)
    rate_next = rate_prev
    # print(rho_t*alpah2, rho_t*alpha3, rho_t*alpha4)
    if runningFastStart and rate_prev != R_max and fg and rate_prev <= alpha1 * rho_t:
        if buf_now['time'] < B_min:
            if rate_plus <= alpah2 * rho_t:
                rate_next = rate_plus
        elif buf_now['time'] < B_low:
            if rate_plus <= alpha3 * rho_t:
                rate_next = rate_plus
        else:
            if rate_plus <= alpha4 * rho_t:
                rate_next = rate_plus
            if buf_now['time'] > B_high:
                B_delay = B_high - phi
    else:
        runningFastStart = False
        if buf_now['time'] < B_min:
            rate_next = R_min
        elif buf_now['time'] < B_low:
            if rate_prev != R_min and rate_prev >= chunk_item[-1]['avg_throughput'] * deltaT / (chunk_item[-1]['end_time'] - chunk_item[-1]['begin_time']):
                rate_next = rate_mins
        elif buf_now['time'] < B_high:
            if rate_prev == R_max or rate_plus >= alpha5 * rho_t:
                B_delay = max(buf_now['time'] - phi, B_opt)
        else:
            if rate_prev == R_max or rate_plus >= alpha5 * rho_t:
                B_delay = max(buf_now['time'] - phi, B_opt)
            else:
                rate_next = rate_plus
                
    # print(runningFastStart, rate_next, B_delay)
    return max(rate_next, R_min), B_delay



    # if buf_now['time'] <= r: #1st check if buffer time is too small, set to R_min
    #     rate_next = R_min
    #     rate_next = match(R_min, R_i)[0]
    # elif buf_now['time'] >= (r + cu):  #too big, set R_max
    #     rate_next = R_max
    #     rate_next = match(R_max, R_i)[0]
    # elif buf_now['current'] >= rate_plus: #check if big enough get a different reasonable rate
    #     less_buff_now= list(i[1] for i in R_i if i[1] < buf_now['current'])
    #     if less_buff_now == []:
    #         rate_next = rate_prev[0]
    #     else: 
    #         rate_next = max(less_buff_now)
    #         rate_next = match(rate_next, R_i)[0]
    # elif buf_now['current'] <= rate_mins: #check if small enough for a different reasonable rate
    #     more_buff_now= list(i[1] for i in R_i if i[1] > buf_now['current'])
    #     if more_buff_now == []:
    #         rate_next = rate_prev[0]
    #     else: 
    #         rate_next = min(more_buff_now)
    #         rate_next = match(rate_next, R_i)[0]
    # else:
    #     rate_next = rate_prev[0] #else give up and try again next time