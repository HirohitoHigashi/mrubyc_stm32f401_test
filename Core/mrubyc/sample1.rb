puts "Start"

while true
  led_write( 1 )
  sleep 0.1
  led_write( 0 )
  sleep 0.1
end

__END__

#pin = GPIO.new("A9", GPIO::OUT)
#p pin
GPIO.setmode("A9", GPIO::OUT)

inp = GPIO.new("A8", GPIO::IN|GPIO::PULL_UP)

while true
  GPIO.write_at("A9", 1 )
  sleep_ms 50
  GPIO.write_at("A9", 0 )
  sleep_ms 500

  p inp.read
end
