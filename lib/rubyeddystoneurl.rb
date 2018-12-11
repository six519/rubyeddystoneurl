=begin
 * 2018 http://ferdinandsilva.com
 *
 * Ruby C Extension To Scan Eddystone-URL Beacons
 * 
 * Author:
 * Ferdinand Silva <ferdinandsilva@ferdinandsilva.com>
 *
=end
require 'rubyeddystoneurl/rubyeddystoneurl' 
include RubyEddystoneURL

def discover(timeout)
    ret = Hash.new

    devices = scan(timeout)

    devices.each do |dev|
        if !ret.key?(dev['address'])
            # add to return
            ret[dev['address']] = {
                "name" => "",
                "url" => "",
                "rssi" => dev['rssi']
            }
        end

        if dev['info'].length > 28
            # valid info
            lineToRemove = dev['info'][0..25]
            cleanLine = dev['info'].sub lineToRemove, ""

            if cleanLine[/^(00|01|02|03)/]
                #URL
                justStarted = true
                urlStr = ""

                while cleanLine.length > 0
                    lineToRemove = cleanLine[0..1]

                    #decode URL
                    if justStarted
                        justStarted = false

                        if lineToRemove == "00"
                            urlStr = "http://www."
                        elsif lineToRemove == "01"
                            urlStr = "https://www."
                        elsif lineToRemove == "02"
                            urlStr = "http://"
                        else
                            # 03
                            urlStr = "https://"
                        end
                        cleanLine = cleanLine.sub lineToRemove, ""
                        next
                    end

                    if lineToRemove == "00"
                        urlStr += ".com/"
                    elsif lineToRemove == "01"
                        urlStr += ".org/"
                    elsif lineToRemove == "02"
                        urlStr += ".edu/"
                    elsif lineToRemove == "03"
                        urlStr += ".net/"
                    elsif lineToRemove == "04"
                        urlStr += ".info/"
                    elsif lineToRemove == "05"
                        urlStr += ".biz/"
                    elsif lineToRemove == "06"
                        urlStr += ".gov/"
                    elsif lineToRemove == "07"
                        urlStr += ".com"
                    elsif lineToRemove == "08"
                        urlStr += ".org"
                    elsif lineToRemove == "09"
                        urlStr += ".edu"
                    elsif lineToRemove == "0a"
                        urlStr += ".net"
                    elsif lineToRemove == "0b"
                        urlStr += ".info"
                    elsif lineToRemove == "0c"
                        urlStr += ".biz"
                    elsif lineToRemove == "0d"
                        urlStr += ".gov"
                    else
                        urlStr += [lineToRemove].pack 'H*'
                    end

                    cleanLine = cleanLine.sub lineToRemove, ""
                end

                ret[dev['address']]['url'] = urlStr
            else
                #name
                lineToRemove = cleanLine[0..3]
                cleanLine = cleanLine.sub lineToRemove, ""
                ret[dev['address']]['name'] = [cleanLine].pack 'H*'
            end
        end
    end

    # clean up devices
    # remove devices that doesn't have a url
    ret.each do |k,v|
        if ret[k]["url"] == ""
            ret.delete k
        end
    end

    return ret
end