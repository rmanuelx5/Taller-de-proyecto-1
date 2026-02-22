import 'package:http/http.dart' as http;

class DispenserService
{
  static const String _baseUrl = 'http://192.168.4.1';

  static Future<bool> dispensarAhora()
  async
  {
    final Uri url = Uri.parse('$_baseUrl/dispensar');

    try
    {
      final respuesta = await http.get(url).timeout(const Duration(seconds: 5));
      return respuesta.statusCode == 200;
    }
    catch (_)
    {
      return false;
    }
  }

  static Future<bool> programarUnico(
      {
        required int hora,
        required int minuto,
      })
  async
  {
    final Uri url = Uri.parse('$_baseUrl/programar?hora=$hora&min=$minuto');

    try
    {
      final respuesta = await http.get(url).timeout(const Duration(seconds: 5));
      return respuesta.statusCode == 200;
    }
    catch (_)
    {
      return false;
    }
  }

  static Future<bool> programarCada(
      {
        required int horas,
        required int minutos,
      })
  async
  {
    final Uri url = Uri.parse(
      '$_baseUrl/cada?h=$horas&min=$minutos',
    );

    try
    {
      final respuesta =
      await http.get(url).timeout(const Duration(seconds: 5));

      return respuesta.statusCode == 200;
    }
    catch (_)
    {
      return false;
    }
  }
}
