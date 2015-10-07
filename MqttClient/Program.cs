using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace MqttClient
{
    class Program
    {
        static void Main(string[] args)
        {

            // create client instance
            uPLibrary.Networking.M2Mqtt.MqttClient client = new uPLibrary.Networking.M2Mqtt.MqttClient(Properties.Settings.Default.BrokerIP);

            // register to message received
            client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;

            string clientId = Guid.NewGuid().ToString();
            client.Connect(clientId);

            // subscribe to the topic "/home/temperature" with QoS 2
            client.Subscribe(new string[] { "#" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
            //client.Publish("ct/1/temp", Encoding.UTF8.GetBytes("20"));

        }

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

        static string DecodeMessage(byte[] message)
        {
            return System.Text.Encoding.UTF8.GetString(message);
        }

        private static List<Models.Application> _applicationList = null;
        public static List<Models.Application> ApplicationList
        {
            get
            {
                if (_applicationList == null)
                {
                    _applicationList = new List<Models.Application>();

                    _applicationList = ctLib.Helpers.JsonHelper.ExecuteREST<List<Models.Application>>(Properties.Settings.Default.ApplicationListURI, null, JSONHeaders, HttpMethod.Get);
                }
                return _applicationList;
            }
        }

        private static List<ctLib.Helpers.JsonHelper.NameValue> _JSONHeaders = null;
        public static List<ctLib.Helpers.JsonHelper.NameValue> JSONHeaders
        {
            get
            {
                if(_JSONHeaders == null)
                {
                    _JSONHeaders = Newtonsoft.Json.JsonConvert.DeserializeObject<List<ctLib.Helpers.JsonHelper.NameValue>>(Properties.Settings.Default.APIHeaders);
                }
                return _JSONHeaders;
            }
        }
        
        public static Models.Application GetApplication(string appID)
        {
            var result = from r in ApplicationList
                         where r.id == appID
                         select r;

            if(result.Count() == 0)
            {
                return null;
            }
            else
            {
                return result.First();
            }
        }

    }
}
