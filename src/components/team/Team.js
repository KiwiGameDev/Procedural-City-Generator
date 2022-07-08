import React from 'react'
import './team.css'
import { StyledEngineProvider } from '@mui/material/styles';

import { Box, Typography, Card, CardActions, CardContent, CardMedia} from '@mui/material'
import {
    createTheme,
    responsiveFontSizes,
    ThemeProvider,
  } from '@mui/material/styles';


let theme = createTheme();
  theme = responsiveFontSizes(theme);

//get image linkm , alt,  and text from props
const Team = (props) => {
  return (
    <StyledEngineProvider injectFirst>
      <ThemeProvider theme={theme}>
          
      <Card sx={{ maxWidth: 345 , margin: "20px 80px", backgroundColor: 'rgba(0,0,0,0.2)'}}>
      <CardMedia
        component="img"
        alt={props.alt}
        src={props.src}
      />
      <CardContent>
        <Typography gutterBottom variant="h5" component="div" sx={{color: 'green'}}>
          {props.text}
        </Typography>
      </CardContent>
    </Card>


        </ThemeProvider>
    </StyledEngineProvider>
  )
}

export default Team