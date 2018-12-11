Gem::Specification.new do |s|
  s.name = "rubyeddystoneurl"
  s.version = "1.0.0"
  s.date = "2018-12-11"
  s.authors = ["Ferdinand E. Silva"]
  s.email = "ferdinandsilva@ferdinandsilva.com"
  s.summary = "Ruby C Extension To Scan Eddystone-URL Bluetooth Low Energy Beacons"
  s.homepage = "http://ferdinandsilva.com"
  s.description = "Ruby C Extension To Scan Eddystone-URL Bluetooth Low Energy Beacons"
  s.files = ["lib/rubyeddystoneurl.rb", "ext/rubyeddystoneurl/extconf.rb", "ext/rubyeddystoneurl/rubyeddystoneurl.c"]
  s.extensions = %w[ext/rubyeddystoneurl/extconf.rb]
end