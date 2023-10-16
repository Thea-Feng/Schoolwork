import socket
import json
import time

# Cut-the-corners TCP Client:
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM | socket.SOCK_CLOEXEC)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.connect(('localhost', 6000))

def send_req_json(m_band, prev_throughput, buf_occ, av_bitrates, current_time, chunk_arg, rebuff_time, pref_bitrate, chunk_item, fg):

    #pack message
    req = json.dumps({"Measured Bandwidth" : m_band,
                     "Previous Throughput" : prev_throughput,
                     "Buffer Occupancy" : buf_occ,
                     "Available Bitrates" : av_bitrates,
                     "Video Time" : current_time,
                     "Chunk" : chunk_arg,
                     "Rebuffering Time" : rebuff_time,
                     "Preferred Bitrate" : pref_bitrate,
                     "Chunk Item": chunk_item,
                     "Flag" : fg,
                     "exit" : 0})
    req += '\n'

    s.sendall(req.encode())

    message = ""
    while(1):
        messagepart = s.recv(2048).decode()

        # print(messagepart)
        message += messagepart
        if message[-1] == '\n':
            # print(message)

            response = json.loads(message)
            # print("highi", response)
            return response["bitrate"], response['B_delay']


def send_exit():
    req = json.dumps({"exit" : 1})

    req += '\n'
    s.sendall(req.encode())


if __name__ == "__main__":
    pass
