# Mqtt .NET Client/Agent
This is an agent that is used to monitor MQTT data and taking certain data types and saving it to an azure datasource

This app will need the M2MQTT client library to operate.  You can get that here: [https://github.com/ppatierno/m2mqtt](https://github.com/ppatierno/m2mqtt)

The main guts of this is the callback for publish received.  This just splits the topic up into an array (based on a split('/').  There should be 4 parts to the topic:


- appid:  this indicates which "application" this is.  Based on that, a particular URI will point to the API to post data to.  this allows us to have multiple applications running through a single broker but save different types of data to different datastores
- direction: this is either a d or an s.  A "d" is data coming from a device.  An "s" is data that is to be sent to the device
- deviceid:  Depending on whether the direction is a "d" or an "s" the deviceid will signify whether data is coming from the deviceid or going to the deviceid
- sensorid:  this is just a variable that represents the message data.  If the data is going to the device (direction = s) then this means that the device will put that data into the variable name.  If it is coming from the device (direction = d) then the server will know that this is a particular sensor or variable that the device wants to send information from.  

        static void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            string[] topicAry = e.Topic.Split('/');

            try
            {
                if (topicAry.Length <= 3)
                {
                    throw new Exception("Topic must have at least <appid>/d/<deviceid>/<sensorid>");
                }

                switch(topicAry[1].ToUpper())
                {
                    case "D":
                        string message = DecodeMessage(e.Message);

                        Models.Application app = GetApplication(topicAry[0]);
                        if (app == null)
                        {
                            throw new Exception("Application for " + topicAry[0] + " Could not be found!");
                        }

                        string returnJSON = ctLib.Helpers.JsonHelper.ExecuteRESTForString(app.apiuri, "{\"deviceid\":" + topicAry[2] + ",\"sensorid\":\"" + topicAry[3] + "\",\"value\":\"" + message + "\"}", JSONHeaders, HttpMethod.Post);

                        Console.WriteLine("topic: " + e.Topic + "\nmessage: " + message);

                        break;

                    default:
                        return;
                }

            }
            catch(Exception ex)
            {
                Console.WriteLine("ERROR:  " + ex.Message);
            }
        }
