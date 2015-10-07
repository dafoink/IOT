using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;


namespace ctLib.Helpers
{
    public class JsonHelper
    {
        public static string JsonSerializer<T>(T t)
        {
            try
            {
                String jsonString = Newtonsoft.Json.JsonConvert.SerializeObject(t);
                return jsonString;
            }
            catch
            {
                return string.Empty;
            }
        }
        public static T JsonDeserialize<T>(string jsonString)
        {
            try
            {
                jsonString = jsonString.Replace("\"custContacts\":null,", "");
                var value = Newtonsoft.Json.JsonConvert.DeserializeObject<T>(jsonString);
                return value;
            }
            catch
            {
                return default(T);
            }
        }

        public enum HTTPMethod { POST, GET, DELETE, PATCH };

        public static T ExecuteREST<T>(string jsonURI, T t, List<NameValue> webRequestHeaders, System.Net.Http.HttpMethod method)
        {
            string jsonPayLoad = "";
            T value = default(T);
            if (t != null)
            {
                jsonPayLoad = JsonSerializer(t);
            }
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(jsonURI);
            request.Method = method.Method;
            request.ContentType = "application/json";
            request.Accept = "application/json";
            request.ContentLength = jsonPayLoad.Length;

            if (webRequestHeaders != null)
            {
                foreach (NameValue item in webRequestHeaders)
                {
                    request.Headers.Add(item.Name, item.Value);
                }
            }

            if (request.ContentLength > 0)
            {
                using (Stream webStream = request.GetRequestStream())
                using (StreamWriter requestWriter = new StreamWriter(webStream, System.Text.Encoding.ASCII))
                {
                    requestWriter.Write(jsonPayLoad);
                }

            }
            jsonPayLoad = jsonPayLoad.Replace("\"custContacts\":null,", "");

            String jsonContent = "";
            try
            {
                WebResponse webResponse = request.GetResponse();
                using (Stream webStream = webResponse.GetResponseStream())
                {
                    if (webStream != null)
                    {
                        using (StreamReader responseReader = new StreamReader(webStream))
                        {
                            jsonContent = responseReader.ReadToEnd();
                            value = JsonDeserialize<T>(jsonContent);
                        }
                    }
                }
            }
            catch (WebException wex)
            {
                if (wex.Response != null)
                {
                    using (var errorResponse = (HttpWebResponse)wex.Response)
                    {
                        using (var reader = new StreamReader(errorResponse.GetResponseStream()))
                        {
                            string error = reader.ReadToEnd();
                            throw new Exception(error);
                        }
                    }

                }
            }
            return value;

        }

        public static string ExecuteRESTForString(string jsonURI, string jsonPayLoad, List<NameValue> webRequestHeaders, System.Net.Http.HttpMethod method)
        {
            if (jsonPayLoad == null) { jsonPayLoad = ""; }
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(jsonURI);
            request.Method = method.Method;
            request.ContentType = "application/json";
            request.ContentLength = jsonPayLoad.Length;

            if (webRequestHeaders != null)
            {
                foreach (NameValue item in webRequestHeaders)
                {
                    request.Headers.Add(item.Name, item.Value);
                }
            }

            if (request.ContentLength > 0)
            {
                using (Stream webStream = request.GetRequestStream())
                using (StreamWriter requestWriter = new StreamWriter(webStream, System.Text.Encoding.ASCII))
                {
                    requestWriter.Write(jsonPayLoad);
                }

            }

            String jsonContent = "";
            try
            {
                WebResponse webResponse = request.GetResponse();
                using (Stream webStream = webResponse.GetResponseStream())
                {
                    if (webStream != null)
                    {
                        using (StreamReader responseReader = new StreamReader(webStream))
                        {
                            jsonContent = responseReader.ReadToEnd();
                        }
                    }
                }
            }
            catch (Exception e)
            {
                throw new Exception("Deserialization Error: " + jsonContent);
            }
            return jsonContent;
        }

        public static string SendToServer(string url, string json, List<NameValue> webRequestHeaders, string method)
        {
            string result = "";
            var request = (HttpWebRequest)WebRequest.Create(url);
            request.Method = method;
            request.ContentType = "application/json";
            request.Accept = "application/json";
            if (webRequestHeaders != null)
            {
                foreach (NameValue item in webRequestHeaders)
                {
                    switch (item.Name.ToLower())
                    {
                        case "range":
                            string[] valueString = item.Value.Split('-');
                            int begin = 0;
                            int end = 0;
                            if (valueString.Length >= 2)
                            {
                                if (!int.TryParse(valueString[0], out begin))
                                {
                                    begin = 0;
                                }
                                if (!int.TryParse(valueString[1], out end))
                                {
                                    end = 0;
                                }
                            }
                            request.AddRange(begin, end);
                            break;

                        default:
                            request.Headers.Add(item.Name, item.Value);
                            break;
                    }
                }
            }
            if (json.Trim() != "")
            {
                using (var streamWriter = new StreamWriter(request.GetRequestStream()))
                {
                    streamWriter.Write(json);
                }
            }
            try
            {
                var response = (HttpWebResponse)request.GetResponse();
                using (var streamReader = new StreamReader(response.GetResponseStream()))
                {
                    result = streamReader.ReadToEnd();
                }
            }
            catch (WebException wex)
            {
                if (wex.Response != null)
                {
                    using (var errorResponse = (HttpWebResponse)wex.Response)
                    {
                        using (var reader = new StreamReader(errorResponse.GetResponseStream()))
                        {
                            string error = reader.ReadToEnd();
                            //throw new Exception(error);
                        }
                    }

                }
            }

            return result;
        }

        public static string GetHeaderValueFromServer(string url, string json, List<NameValue> webRequestHeaders, string method, string returnHeaderName)
        {
            string result = "";
            var request = (HttpWebRequest)WebRequest.Create(url);
            request.Method = method;
            request.ContentType = "application/json";
            request.Accept = "application/json";
            if (webRequestHeaders != null)
            {
                foreach (NameValue item in webRequestHeaders)
                {
                    request.Headers.Add(item.Name, item.Value);
                }
            }
            if (json.Trim() != "")
            {
                using (var streamWriter = new StreamWriter(request.GetRequestStream()))
                {
                    streamWriter.Write(json);
                }
            }
            try
            {
                var response = (HttpWebResponse)request.GetResponse();
                result = response.Headers[returnHeaderName];

            }
            catch (WebException wex)
            {
                if (wex.Response != null)
                {
                    using (var errorResponse = (HttpWebResponse)wex.Response)
                    {
                        using (var reader = new StreamReader(errorResponse.GetResponseStream()))
                        {
                            string error = reader.ReadToEnd();
                            //throw new Exception(error);
                        }
                    }

                }
            }

            return result;
        }
        public static string SendToServer<T>(string url, T t, List<NameValue> webRequestHeaders, string method)
        {
            string json = JsonSerializer(t);
            json = json.Replace("\"custContacts\":null,", "");
            json = json.Replace("\"businessRuleViolations\":null,", "");
            return SendToServer(url, json, webRequestHeaders, method);


        }

        public class NameValue
        {
            public string Name { get; set; }
            public string Value { get; set; }
        }
    }
}
