import 'package:flutter/material.dart';

class PantallaSecundaria extends StatelessWidget
{
  const PantallaSecundaria({super.key});

  @override
  Widget build(BuildContext context)
  {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Pantalla secundaria'),
      ),
      body: Center(
        child: ElevatedButton(
          onPressed: ()
          {
            Navigator.pop(context);
          },
          child: const Text("Volver a inicio"),
        ),
      ),
    );
  }
}