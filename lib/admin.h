#define ADMIN_PAGE "/ngx-stats"
#define ADMIN_PAGE_LEN 9
#define ADMIN_HTML "<!DOCTYPE html> <html> <script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js\"></script> <script src=\"https://cdn.jsdelivr.net/npm/apexcharts\"></script> <body> <html> <head> <title>ngx-stats</title> <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/font-awesome/4.4.0/css/font-awesome.min.css\"> <link href=\"https://fonts.googleapis.com/css?family=Open+Sans:400,700\" rel=\"stylesheet\"> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.6.2/dist/css/bootstrap.min.css\"> <script src=\"https://cdn.jsdelivr.net/npm/jquery@3.6.1/dist/jquery.slim.min.js\"></script> <script src=\"https://cdn.jsdelivr.net/npm/popper.js@1.16.1/dist/umd/popper.min.js\"></script> <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@4.6.2/dist/js/bootstrap.bundle.min.js\"></script> <style> body { font-family: \"Open Sans\", sans-serif; background-color: #F0F4FC; } .container-main { margin: 0 auto; background-color: #fff; max-width: 1700px; margin-top: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); } </style> </head> <body> <nav class=\"navbar navbar-expand-sm bg-dark navbar-dark\"> <a class=\"navbar-brand\" href=\"#\"> <img src=\"\" alt=\"ngx-stats\" style=\"width:40px;\"> </a> <div id=\"nav-links\" style=\"display: none;\"> <ul class=\"nav navbar-nav\"> <li class=\"nav-item\"> <a class=\"nav-link\" href=\"#\">Link 1</a> </li> <li class=\"nav-item\"> <a class=\"nav-link\" href=\"#\">Link 2</a> </li> <li class=\"nav-item\"> <a class=\"nav-link\" href=\"#\">Link 3</a> </li> </ul> </div> <ul class=\"navbar-nav ml-auto\" style=\"display: none;\"> <li class=\"nav-item\"> <a class=\"nav-link\" href=\"#\" onclick=\"doSignOut()\"> <i class=\"fa fa-sign-out\"></i> Sign out </a> </li> </ul> </nav> <div id=\"main-page\"> <div class=\"container-main container\"> <div class=\"row\"> <div class=\"col-sm\" style=\"height: 300px\"> <canvas id=\"myChart\" style=\"max-width:100vw;\"></canvas> </div> </div> </body> </html> <script> var xValues = [ new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString(), new Date(%u*1000).toLocaleTimeString()]; var yValues = [ %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu]; chart = new Chart(\"myChart\", { type: \"line\", data: { labels: xValues, datasets: [{ fill: true, label: \"r/s\", backgroundColor: \"rgba(34,139,34,0.7)\", borderColor: \"rgba(34,139,34,0.1)\", data: yValues }] }, options: { responsive: true, maintainAspectRatio: false, legend: { display: true, text: \"r/s\" }, scales: { yAxes: [{ ticks: { min: 0, max: 16 } }], }, animation: false, scaleOverride: true, scaleSteps: 10, scaleStepWidth: 10, scaleStartValue: 0 } }); chart.legend.legendItems[0].text = \"r/s\"; function update_r_p_s(r_p_s, time) { let max = 0; if ( chart.data.datasets[0].data.length < (60*60+2) ){ chart.data.datasets[0].data.push(r_p_s); chart.data.labels.push( new Date(time*1000).toLocaleTimeString() ); for (let i = 0; i < chart.data.datasets[0].data.length - 1; i++) { if (chart.data.datasets[0].data[i] > max) { max = chart.data.datasets[0].data[i]; } } }else{ for (let i = 0; i < chart.data.datasets[0].data.length - 1; i++) { chart.data.datasets[0].data[i] = chart.data.datasets[0].data[i + 1]; chart.data.labels[i] = chart.data.labels[i+1]; if (chart.data.datasets[0].data[i] > max) { max = chart.data.datasets[0].data[i]; } } chart.data.datasets[0].data[chart.data.datasets[0].data.length - 1] = Math.ceil(Math.random() * 100); chart.data.datasets[0].data[chart.data.datasets[0].data.length - 1] = r_p_s; chart.data.labels[chart.data.labels.length -1] = new Date(time*1000).toLocaleTimeString() ; } if (chart.data.datasets[0].data[chart.data.datasets[0].data.length - 1] > max) { max = chart.data.datasets[0].data[chart.data.datasets[0].data.length - 1]; } if (max < 5) { max = 5; } chart.options.scales.yAxes[0].ticks.max = max + ( max * Math.floor(0.35) ); chart.update(); } function load_r_p_s(url) { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { let responseString = this.responseText.split(\"}\"); obj = JSON.parse(responseString[0] + \"}\"); update_r_p_s(obj.req, obj.time); } }; xhttp.open(\"GET\", url, true); xhttp.send(); }  var updateChart = window.setInterval(function() { load_r_p_s(\"/ngx-stats?r_p_s=1\"); }, 1000); </script> </body> </html> "
#define get_route_p_s "/ngx-stats?r_p_s=1"  

int is_admin(ngx_http_request_t* r, char* admin_ip){

    if ( (r->uri.len - 1 )< ADMIN_PAGE_LEN )
        return 0;

    if ( strncmp ( admin_ip, (char *)r->connection->addr_text.data, r->connection->addr_text.len  ) != 0)
        return 0;

    if (strncmp ((char*)r->uri.data, get_route_p_s, 18) == 0)
        return 2;

    if (r->uri.len - 2  == ADMIN_PAGE_LEN && r->uri.data[ADMIN_PAGE_LEN+1] != '/' )
        return 0;

    if ( (r->uri.len - 2) > ADMIN_PAGE_LEN && r->uri.data[ADMIN_PAGE_LEN+1] != '?')
        return 0;
    
    if (strncmp ((char*)r->uri.data, ADMIN_PAGE, ADMIN_PAGE_LEN + 1) != 0)
        return 0;

    return 1;
}

