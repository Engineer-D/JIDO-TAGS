from django.shortcuts import render
from django.http import HttpResponse, HttpResponseRedirect, JsonResponse
from django.contrib.auth.decorators import login_required
from django.views.decorators.csrf import csrf_exempt
from django.core.urlresolvers import reverse
from django.forms.models import model_to_dict
from sensors.models import Sensor, SensorData, Actuator

from .models import Plant


# Create your views here.
@login_required(login_url='/')
@csrf_exempt
def add_plant(request):
    if request.method == 'POST':
        plant = Plant()
        if (not request.POST.get('alias')) or len(Plant.objects.filter(alias=request.POST.get('alias'))) != 0:
            return HttpResponseRedirect(reverse('plants:dashboard'))
        plant.alias = request.POST.get('alias')
        plant.parent = request.user
        plant.save()
        sensor1 = Sensor()
        sensor1.sensor_type = 'GPS Module'
        sensor1.parent = plant
        sensor1.save()
        actuator = Actuator()
        actuator.parent = plant
        actuator.name = request.POST.get('alias')
        actuator.state = 0
        actuator.save()

        return HttpResponseRedirect(reverse('plants:dashboard'))
    return HttpResponseRedirect(reverse('plants:dashboard'))


@login_required(login_url='/')
def viewdashboard(request):
    print(request.user)
    plants = Plant.objects.filter(parent=request.user)
    print(plants)
    return render(request, 'userdashboard.html', {'username': request.user.username, 'plants': plants})


@login_required(login_url='/')
def last_readings(request, username):
    print(username)
    plant = Plant.objects.get(alias=username, parent=request.user)
    print(plant.alias)
    sensors = plant.sensor_set.all()
    gps_values = SensorData.objects.filter(parent__sensor_type='GPS Module', parent__parent=plant)[:100]
    gps_values = list(map(lambda x: model_to_dict(x), gps_values))

    dict1 = {'gps_values': gps_values}
    return JsonResponse(dict1, safe=False)


@login_required(login_url='/')
def plantboard(request, username):
    print(username)
    plant = Plant.objects.get(alias=username, parent=request.user)
    print(plant.alias)
    sensors = plant.sensor_set.all()
    print(sensors)
    gps_sensor = sensors.filter(sensor_type='GPS Module')[0]
    sensor_data_gps = SensorData.objects.filter(parent=gps_sensor)
    act = Actuator.objects.get(parent__alias=username, name=plant.alias)
    try:
        gps = sensor_data_gps.latest('id')
    except Exception:
        gps = None

    gps_values = list(map(lambda x: x.value, list(sensor_data_gps)[-1:-11:-1]))

    context = {'plant': plant,
               'sensors': sensors,
               'location': gps,
               'gps_values': gps_values[::-1]
               }
    print(context)
    return render(request, "plant1.html", context=context)
    return HttpResponse(str(id))
