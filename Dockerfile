FROM gcc:14

WORKDIR /app

COPY . .

RUN gcc backend/server.c backend/mongoose.c patient.c doctor.c appointment.c room.c billing.c -o medcare_server

EXPOSE 10000

CMD ["./medcare_server"]
